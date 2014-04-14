package main

import (
	"bytes"
	"crypto/md5"
	"database/sql"
	"encoding/base64"
	"encoding/json"
	"flag"
	"fmt"
	_ "github.com/go-sql-driver/mysql"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"text/template"
)

const (
	RESULT_COMPILE = `c++ -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_SCALAR_TO_FLOAT_EXCLUDED -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SUPPORT_GPU=0 -DSK_SUPPORT_OPENCL=0 -DSK_FORCE_DISTANCEFIELD_FONTS=0 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SAMPLES_FOR_X -DSK_BUILD_FOR_UNIX -DSK_USE_POSIX_THREADS -DSK_SYSTEM_ZLIB=1 -DSK_DEBUG -DSK_DEVELOPER=1 -I../../src/core -I../../src/images -I../../tools/flags -I../../include/config -I../../include/core -I../../include/pathops -I../../include/pipe -I../../include/effects -I../../include/ports -I../../src/sfnt -I../../include/utils -I../../src/utils -I../../include/images -g -fno-exceptions -fstrict-aliasing -Wall -Wextra -Winit-self -Wpointer-arith -Wno-unused-parameter -Wno-c++11-extensions -Werror -m64 -fno-rtti -Wnon-virtual-dtor -c ../../../cache/%s.cpp -o ../../../cache/%s.o`
	LINK           = `c++ -m64 -lstdc++ -lm -o ../../../inout/%s -Wl,--start-group ../../../cache/%s.o obj/experimental/webtry/webtry.main.o obj/gyp/libflags.a libskia_images.a libskia_core.a libskia_effects.a obj/gyp/libjpeg.a obj/gyp/libwebp_dec.a obj/gyp/libwebp_demux.a obj/gyp/libwebp_dsp.a obj/gyp/libwebp_enc.a obj/gyp/libwebp_utils.a libskia_utils.a libskia_opts.a libskia_opts_ssse3.a libskia_ports.a libskia_sfnt.a -Wl,--end-group -lpng -lz -lgif -lpthread -lfontconfig -ldl -lfreetype`
)

var (
	// codeTemplate is the cpp code template the user's code is copied into.
	codeTemplate *template.Template = nil

	// index is the main index.html page we serve.
	index []byte

	// db is the database, nil if we don't have an SQL database to store data into.
	db *sql.DB = nil
)

// flags
var (
	useChroot = flag.Bool("use_chroot", false, "Run the compiled code in the schroot jail.")
	port      = flag.String("port", ":8000", "HTTP service address (e.g., ':8000')")
)

// lineNumbers adds #line numbering to the user's code.
func LineNumbers(c string) string {
	lines := strings.Split(c, "\n")
	ret := []string{}
	for i, line := range lines {
		ret = append(ret, fmt.Sprintf("#line %d", i+1))
		ret = append(ret, line)
	}
	return strings.Join(ret, "\n")
}

func init() {
	// Change the current working directory to the directory of the executable.
	var err error
	cwd, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		log.Fatal(err)
	}
	os.Chdir(cwd)

	codeTemplate, err = template.ParseFiles(filepath.Join(cwd, "templates/template.cpp"))
	if err != nil {
		panic(err)
	}
	index, err = ioutil.ReadFile(filepath.Join(cwd, "templates/index.html"))
	if err != nil {
		panic(err)
	}

	// Connect to MySQL server. First, get the password from the metadata server.
	// See https://developers.google.com/compute/docs/metadata#custom.
	req, err := http.NewRequest("GET", "http://metadata/computeMetadata/v1/instance/attributes/password", nil)
	if err != nil {
		panic(err)
	}
	client := http.Client{}
	req.Header.Add("X-Google-Metadata-Request", "True")
	if resp, err := client.Do(req); err == nil {
		password, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			log.Printf("ERROR: Failed to read password from metadata server: %q\n", err)
			panic(err)
		}
		// The IP address of the database is found here:
		//    https://console.developers.google.com/project/31977622648/sql/instances/webtry/overview
		// And 3306 is the default port for MySQL.
		db, err = sql.Open("mysql", fmt.Sprintf("webtry:%s@tcp(173.194.83.52:3306)/webtry", password))
		if err != nil {
			log.Printf("ERROR: Failed to open connection to SQL server: %q\n", err)
			panic(err)
		}
	} else {
		log.Printf("INFO: Failed to find metadata, unable to connect to MySQL server (Expected when running locally): %q\n", err)
	}
}

// userCode is used in template expansion.
type userCode struct {
	UserCode string
}

// expandToFile expands the template and writes the result to the file.
func expandToFile(filename string, code string, t *template.Template) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	return t.Execute(f, struct{ UserCode string }{UserCode: code})
}

// expandCode expands the template into a file and calculate the MD5 hash.
func expandCode(code string) (string, error) {
	h := md5.New()
	h.Write([]byte(code))
	hash := fmt.Sprintf("%x", h.Sum(nil))
	// At this point we are running in skia/experimental/webtry, making cache a
	// peer directory to skia.
	// TODO(jcgregorio) Make all relative directories into flags.
	err := expandToFile(fmt.Sprintf("../../../cache/%s.cpp", hash), code, codeTemplate)
	return hash, err
}

// response is serialized to JSON as a response to POSTs.
type response struct {
	Message string `json:"message"`
	Img     string `json:"img"`
}

// doCmd executes the given command line string in either the out/Debug
// directory or the inout directory. Returns the stdout, and stderr in the case
// of a non-zero exit code.
func doCmd(commandLine string, moveToDebug bool) (string, error) {
	log.Printf("Command: %q\n", commandLine)
	programAndArgs := strings.SplitN(commandLine, " ", 2)
	program := programAndArgs[0]
	args := []string{}
	if len(programAndArgs) > 1 {
		args = strings.Split(programAndArgs[1], " ")
	}
	cmd := exec.Command(program, args...)
	abs, err := filepath.Abs("../../out/Debug")
	if err != nil {
		return "", fmt.Errorf("Failed to find absolute path to Debug directory.")
	}
	if moveToDebug {
		cmd.Dir = abs
	} else if !*useChroot { // Don't set cmd.Dir when using chroot.
		abs, err := filepath.Abs("../../../inout")
		if err != nil {
			return "", fmt.Errorf("Failed to find absolute path to inout directory.")
		}
		cmd.Dir = abs
	}
	log.Printf("Run in directory: %q\n", cmd.Dir)
	var stdOut bytes.Buffer
	cmd.Stdout = &stdOut
	var stdErr bytes.Buffer
	cmd.Stderr = &stdErr
	cmd.Start()
	err = cmd.Wait()
	message := stdOut.String()
	log.Printf("StdOut: %s\n", message)
	if err != nil {
		log.Printf("Exit status: %s\n", err.Error())
		log.Printf("StdErr: %s\n", stdErr.String())
		message += stdErr.String()
		return message, fmt.Errorf("Failed to run command.")
	}
	return message, nil
}

// reportError formats an HTTP error response and also logs the detailed error message.
func reportError(w http.ResponseWriter, r *http.Request, err error, message string) {
	m := response{
		Message: message,
	}
	log.Printf("Error: %s\n%s", message, err.Error())
	resp, err := json.Marshal(m)
	if err != nil {
		http.Error(w, "Failed to serialize a response", 500)
		return
	}
	w.Write(resp)
}

func writeToDatabase(hash string, code string) {
	if db == nil {
		return
	}
	if _, err := db.Exec("INSERT INTO webtry (code, hash) VALUES(?, ?)", code, hash); err != nil {
		log.Printf("ERROR: Failed to insert code into database: %q\n", err)
	}
}

// mainHandler handles the GET and POST of the main page.
func mainHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method == "GET" {
		w.Write(index)
	} else if r.Method == "POST" {
		w.Header().Set("Content-Type", "application/json")
		b, err := ioutil.ReadAll(r.Body)
		if err != nil {
			reportError(w, r, err, "Failed to read a request body.")
			return
		}
		code := string(b)
		hash, err := expandCode(LineNumbers(code))
		if err != nil {
			reportError(w, r, err, "Failed to write the code to compile.")
			return
		}
		writeToDatabase(hash, code)
		message, err := doCmd(fmt.Sprintf(RESULT_COMPILE, hash, hash), true)
		if err != nil {
			reportError(w, r, err, "Failed to compile the code:\n"+message)
			return
		}
		linkMessage, err := doCmd(fmt.Sprintf(LINK, hash, hash), true)
		if err != nil {
			reportError(w, r, err, "Failed to link the code:\n"+linkMessage)
			return
		}
		message += linkMessage
		cmd := hash + " --out " + hash + ".png"
		if *useChroot {
			cmd = "schroot -c webtry --directory=/inout -- /inout/" + cmd
		} else {
			abs, err := filepath.Abs("../../../inout")
			if err != nil {
				reportError(w, r, err, "Failed to find executable directory.")
				return
			}
			cmd = abs + "/" + cmd
		}

		execMessage, err := doCmd(cmd, false)
		if err != nil {
			reportError(w, r, err, "Failed to run the code:\n"+execMessage)
			return
		}
		png, err := ioutil.ReadFile("../../../inout/" + hash + ".png")
		if err != nil {
			reportError(w, r, err, "Failed to open the generated PNG.")
			return
		}

		m := response{
			Message: message,
			Img:     base64.StdEncoding.EncodeToString([]byte(png)),
		}
		resp, err := json.Marshal(m)
		if err != nil {
			reportError(w, r, err, "Failed to serialize a response.")
			return
		}
		w.Write(resp)
	}
}

func main() {
	flag.Parse()

	http.HandleFunc("/", mainHandler)
	log.Fatal(http.ListenAndServe(*port, nil))
}
