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
	_ "github.com/mattn/go-sqlite3"
	htemplate "html/template"
	"io/ioutil"
	"log"
	"math/rand"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
	"text/template"
	"time"
)

const (
	RESULT_COMPILE = `c++ -DSK_GAMMA_SRGB -DSK_GAMMA_APPLY_TO_A8 -DSK_SCALAR_TO_FLOAT_EXCLUDED -DSK_ALLOW_STATIC_GLOBAL_INITIALIZERS=1 -DSK_SUPPORT_GPU=0 -DSK_SUPPORT_OPENCL=0 -DSK_FORCE_DISTANCEFIELD_FONTS=0 -DSK_SCALAR_IS_FLOAT -DSK_CAN_USE_FLOAT -DSK_SAMPLES_FOR_X -DSK_BUILD_FOR_UNIX -DSK_USE_POSIX_THREADS -DSK_SYSTEM_ZLIB=1 -DSK_DEBUG -DSK_DEVELOPER=1 -I../../src/core -I../../src/images -I../../tools/flags -I../../include/config -I../../include/core -I../../include/pathops -I../../include/pipe -I../../include/effects -I../../include/ports -I../../src/sfnt -I../../include/utils -I../../src/utils -I../../include/images -g -fno-exceptions -fstrict-aliasing -Wall -Wextra -Winit-self -Wpointer-arith -Wno-unused-parameter -Wno-c++11-extensions -Werror -m64 -fno-rtti -Wnon-virtual-dtor -c ../../../cache/%s.cpp -o ../../../cache/%s.o`
	LINK           = `c++ -m64 -lstdc++ -lm -o ../../../inout/%s -Wl,--start-group ../../../cache/%s.o obj/experimental/webtry/webtry.main.o obj/gyp/libflags.a libskia_images.a libskia_core.a libskia_effects.a obj/gyp/libjpeg.a obj/gyp/libwebp_dec.a obj/gyp/libwebp_demux.a obj/gyp/libwebp_dsp.a obj/gyp/libwebp_enc.a obj/gyp/libwebp_utils.a libskia_utils.a libskia_opts.a libskia_opts_ssse3.a libskia_ports.a libskia_sfnt.a -Wl,--end-group -lpng -lz -lgif -lpthread -lfontconfig -ldl -lfreetype`
	DEFAULT_SAMPLE = `SkPaint p;
p.setColor(SK_ColorRED);
p.setAntiAlias(true);
p.setStyle(SkPaint::kStroke_Style);
p.setStrokeWidth(10);

canvas->drawLine(20, 20, 100, 100, p);
`
	// Don't increase above 2^16 w/o altering the db tables to accept something bigger than TEXT.
	MAX_TRY_SIZE = 64000
)

var (
	// codeTemplate is the cpp code template the user's code is copied into.
	codeTemplate *template.Template = nil

	// indexTemplate is the main index.html page we serve.
	indexTemplate *htemplate.Template = nil

	// iframeTemplate is the main index.html page we serve.
	iframeTemplate *htemplate.Template = nil

	// recentTemplate is a list of recent images.
	recentTemplate *htemplate.Template = nil

	// workspaceTemplate is the page for workspaces, a series of webtrys.
	workspaceTemplate *htemplate.Template = nil

	// db is the database, nil if we don't have an SQL database to store data into.
	db *sql.DB = nil

	// directLink is the regex that matches URLs paths that are direct links.
	directLink = regexp.MustCompile("^/c/([a-f0-9]+)$")

	// iframeLink is the regex that matches URLs paths that are links to iframes.
	iframeLink = regexp.MustCompile("^/iframe/([a-f0-9]+)$")

	// imageLink is the regex that matches URLs paths that are direct links to PNGs.
	imageLink = regexp.MustCompile("^/i/([a-f0-9]+.png)$")

	// workspaceLink is the regex that matches URLs paths for workspaces.
	workspaceLink = regexp.MustCompile("^/w/([a-z0-9-]+)$")

	// workspaceNameAdj is a list of adjectives for building workspace names.
	workspaceNameAdj = []string{
		"autumn", "hidden", "bitter", "misty", "silent", "empty", "dry", "dark",
		"summer", "icy", "delicate", "quiet", "white", "cool", "spring", "winter",
		"patient", "twilight", "dawn", "crimson", "wispy", "weathered", "blue",
		"billowing", "broken", "cold", "damp", "falling", "frosty", "green",
		"long", "late", "lingering", "bold", "little", "morning", "muddy", "old",
		"red", "rough", "still", "small", "sparkling", "throbbing", "shy",
		"wandering", "withered", "wild", "black", "young", "holy", "solitary",
		"fragrant", "aged", "snowy", "proud", "floral", "restless", "divine",
		"polished", "ancient", "purple", "lively", "nameless",
	}

	// workspaceNameNoun is a list of nouns for building workspace names.
	workspaceNameNoun = []string{
		"waterfall", "river", "breeze", "moon", "rain", "wind", "sea", "morning",
		"snow", "lake", "sunset", "pine", "shadow", "leaf", "dawn", "glitter",
		"forest", "hill", "cloud", "meadow", "sun", "glade", "bird", "brook",
		"butterfly", "bush", "dew", "dust", "field", "fire", "flower", "firefly",
		"feather", "grass", "haze", "mountain", "night", "pond", "darkness",
		"snowflake", "silence", "sound", "sky", "shape", "surf", "thunder",
		"violet", "water", "wildflower", "wave", "water", "resonance", "sun",
		"wood", "dream", "cherry", "tree", "fog", "frost", "voice", "paper",
		"frog", "smoke", "star",
	}
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
	rand.Seed(time.Now().UnixNano())

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
	indexTemplate, err = htemplate.ParseFiles(
		filepath.Join(cwd, "templates/index.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
	)
	if err != nil {
		panic(err)
	}
	iframeTemplate, err = htemplate.ParseFiles(
		filepath.Join(cwd, "templates/iframe.html"),
	)
	if err != nil {
		panic(err)
	}
	recentTemplate, err = htemplate.ParseFiles(
		filepath.Join(cwd, "templates/recent.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
	)
	if err != nil {
		panic(err)
	}
	workspaceTemplate, err = htemplate.ParseFiles(
		filepath.Join(cwd, "templates/workspace.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
	)
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
		db, err = sql.Open("mysql", fmt.Sprintf("webtry:%s@tcp(173.194.83.52:3306)/webtry?parseTime=true", password))
		if err != nil {
			log.Printf("ERROR: Failed to open connection to SQL server: %q\n", err)
			panic(err)
		}
	} else {
		log.Printf("INFO: Failed to find metadata, unable to connect to MySQL server (Expected when running locally): %q\n", err)
		// Fallback to sqlite for local use.
		db, err = sql.Open("sqlite3", "./webtry.db")
		if err != nil {
			log.Printf("ERROR: Failed to open: %q\n", err)
			panic(err)
		}
		sql := `CREATE TABLE webtry (
             code      TEXT      DEFAULT ''                 NOT NULL,
             create_ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
             hash      CHAR(64)  DEFAULT ''                 NOT NULL,
             PRIMARY KEY(hash)
            )`
		_, err = db.Exec(sql)
		log.Printf("Info: status creating sqlite table for webtry: %q\n", err)
		sql = `CREATE TABLE workspace (
          name      CHAR(64)  DEFAULT ''                 NOT NULL,
          create_ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
          PRIMARY KEY(name)
        )`
		_, err = db.Exec(sql)
		log.Printf("Info: status creating sqlite table for workspace: %q\n", err)
		sql = `CREATE TABLE workspacetry (
          name      CHAR(64)  DEFAULT ''                 NOT NULL,
          create_ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
          hash      CHAR(64)  DEFAULT ''                 NOT NULL,
          hidden    INTEGER   DEFAULT 0                  NOT NULL,

          FOREIGN KEY (name) REFERENCES workspace(name)
        )`
		_, err = db.Exec(sql)
		log.Printf("Info: status creating sqlite table for workspace try: %q\n", err)
	}
}

// userCode is used in template expansion.
type userCode struct {
	UserCode string
	Hash     string
}

// expandToFile expands the template and writes the result to the file.
func expandToFile(filename string, code string, t *template.Template) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	return t.Execute(f, userCode{UserCode: code})
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
	Hash    string `json:"hash"`
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

func writeToDatabase(hash string, code string, workspaceName string) {
	if db == nil {
		return
	}
	if _, err := db.Exec("INSERT INTO webtry (code, hash) VALUES(?, ?)", code, hash); err != nil {
		log.Printf("ERROR: Failed to insert code into database: %q\n", err)
	}
	if workspaceName != "" {
		if _, err := db.Exec("INSERT INTO workspacetry (name, hash) VALUES(?, ?)", workspaceName, hash); err != nil {
			log.Printf("ERROR: Failed to insert into workspacetry table: %q\n", err)
		}
	}
}

func cssHandler(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "css/webtry.css")
}

func jsHandler(w http.ResponseWriter, r *http.Request) {
	http.ServeFile(w, r, "js/run.js")
}

// imageHandler serves up the PNG of a specific try.
func imageHandler(w http.ResponseWriter, r *http.Request) {
	log.Printf("Image Handler: %q\n", r.URL.Path)
	if r.Method != "GET" {
		http.NotFound(w, r)
		return
	}
	match := imageLink.FindStringSubmatch(r.URL.Path)
	if len(match) != 2 {
		http.NotFound(w, r)
		return
	}
	filename := match[1]
	http.ServeFile(w, r, fmt.Sprintf("../../../inout/%s", filename))
}

type Try struct {
	Hash     string
	CreateTS string
}

type Recent struct {
	Tries []Try
}

// recentHandler shows the last 20 tries.
func recentHandler(w http.ResponseWriter, r *http.Request) {
	log.Printf("Recent Handler: %q\n", r.URL.Path)

	var err error
	rows, err := db.Query("SELECT create_ts, hash FROM webtry ORDER BY create_ts DESC LIMIT 20")
	if err != nil {
		http.NotFound(w, r)
		return
	}
	recent := []Try{}
	for rows.Next() {
		var hash string
		var create_ts time.Time
		if err := rows.Scan(&create_ts, &hash); err != nil {
			log.Printf("Error: failed to fetch from database: %q", err)
			continue
		}
		recent = append(recent, Try{Hash: hash, CreateTS: create_ts.Format("2006-02-01")})
	}
	if err := recentTemplate.Execute(w, Recent{Tries: recent}); err != nil {
		log.Printf("ERROR: Failed to expand template: %q\n", err)
	}
}

type Workspace struct {
	Name  string
	Code  string
	Tries []Try
}

// newWorkspace generates a new random workspace name and stores it in the database.
func newWorkspace() (string, error) {
	for i := 0; i < 10; i++ {
		adj := workspaceNameAdj[rand.Intn(len(workspaceNameAdj))]
		noun := workspaceNameNoun[rand.Intn(len(workspaceNameNoun))]
		suffix := rand.Intn(1000)
		name := fmt.Sprintf("%s-%s-%d", adj, noun, suffix)
		if _, err := db.Exec("INSERT INTO workspace (name) VALUES(?)", name); err == nil {
			return name, nil
		} else {
			log.Printf("ERROR: Failed to insert workspace into database: %q\n", err)
		}
	}
	return "", fmt.Errorf("Failed to create a new workspace")
}

// getCode returns the code for a given hash, or the empty string if not found.
func getCode(hash string) string {
	code := ""
	if err := db.QueryRow("SELECT code FROM webtry WHERE hash=?", hash).Scan(&code); err != nil {
		log.Printf("ERROR: Code for hash is missing: %q\n", err)
	}
	return code
}

func workspaceHandler(w http.ResponseWriter, r *http.Request) {
	log.Printf("Workspace Handler: %q\n", r.URL.Path)
	if r.Method == "GET" {
		tries := []Try{}
		match := workspaceLink.FindStringSubmatch(r.URL.Path)
		name := ""
		if len(match) == 2 {
			name = match[1]
			rows, err := db.Query("SELECT create_ts, hash FROM workspacetry WHERE name=? ORDER BY create_ts DESC ", name)
			if err != nil {
				reportError(w, r, err, "Failed to select.")
				return
			}
			for rows.Next() {
				var hash string
				var create_ts time.Time
				if err := rows.Scan(&create_ts, &hash); err != nil {
					log.Printf("Error: failed to fetch from database: %q", err)
					continue
				}
				tries = append(tries, Try{Hash: hash, CreateTS: create_ts.Format("2006-02-01")})
			}
		}
		var code string
		if len(tries) == 0 {
			code = DEFAULT_SAMPLE
		} else {
			code = getCode(tries[0].Hash)
		}
		if err := workspaceTemplate.Execute(w, Workspace{Tries: tries, Code: code, Name: name}); err != nil {
			log.Printf("ERROR: Failed to expand template: %q\n", err)
		}
	} else if r.Method == "POST" {
		name, err := newWorkspace()
		if err != nil {
			http.Error(w, "Failed to create a new workspace.", 500)
			return
		}
		http.Redirect(w, r, "/w/"+name, 302)
	}
}

// hasPreProcessor returns true if any line in the code begins with a # char.
func hasPreProcessor(code string) bool {
	lines := strings.Split(code, "\n")
	for _, s := range lines {
		if strings.HasPrefix(strings.TrimSpace(s), "#") {
			return true
		}
	}
	return false
}

type TryRequest struct {
	Code string `json:"code"`
	Name string `json:"name"`
}

// iframeHandler handles the GET and POST of the main page.
func iframeHandler(w http.ResponseWriter, r *http.Request) {
	log.Printf("IFrame Handler: %q\n", r.URL.Path)
	if r.Method != "GET" {
		http.NotFound(w, r)
		return
	}
	match := iframeLink.FindStringSubmatch(r.URL.Path)
	if len(match) != 2 {
		http.NotFound(w, r)
		return
	}
	hash := match[1]
	if db == nil {
		http.NotFound(w, r)
		return
	}
	var code string
	// Load 'code' with the code found in the database.
	if err := db.QueryRow("SELECT code FROM webtry WHERE hash=?", hash).Scan(&code); err != nil {
		http.NotFound(w, r)
		return
	}
	// Expand the template.
	if err := iframeTemplate.Execute(w, userCode{UserCode: code, Hash: hash}); err != nil {
		log.Printf("ERROR: Failed to expand template: %q\n", err)
	}
}

// mainHandler handles the GET and POST of the main page.
func mainHandler(w http.ResponseWriter, r *http.Request) {
	log.Printf("Main Handler: %q\n", r.URL.Path)
	if r.Method == "GET" {
		code := DEFAULT_SAMPLE
		match := directLink.FindStringSubmatch(r.URL.Path)
		if len(match) == 2 && r.URL.Path != "/" {
			hash := match[1]
			if db == nil {
				http.NotFound(w, r)
				return
			}
			// Update 'code' with the code found in the database.
			if err := db.QueryRow("SELECT code FROM webtry WHERE hash=?", hash).Scan(&code); err != nil {
				http.NotFound(w, r)
				return
			}
		}
		// Expand the template.
		if err := indexTemplate.Execute(w, userCode{UserCode: code}); err != nil {
			log.Printf("ERROR: Failed to expand template: %q\n", err)
		}
	} else if r.Method == "POST" {
		w.Header().Set("Content-Type", "application/json")
		buf := bytes.NewBuffer(make([]byte, 0, MAX_TRY_SIZE))
		n, err := buf.ReadFrom(r.Body)
		if err != nil {
			reportError(w, r, err, "Failed to read a request body.")
			return
		}
		if n == MAX_TRY_SIZE {
			err := fmt.Errorf("Code length equal to, or exceeded, %d", MAX_TRY_SIZE)
			reportError(w, r, err, "Code too large.")
			return
		}
		request := TryRequest{}
		if err := json.Unmarshal(buf.Bytes(), &request); err != nil {
			reportError(w, r, err, "Coulnd't decode JSON.")
			return
		}
		if hasPreProcessor(request.Code) {
			err := fmt.Errorf("Found preprocessor macro in code.")
			reportError(w, r, err, "Preprocessor macros aren't allowed.")
			return
		}
		hash, err := expandCode(LineNumbers(request.Code))
		if err != nil {
			reportError(w, r, err, "Failed to write the code to compile.")
			return
		}
		writeToDatabase(hash, request.Code, request.Name)
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
			Hash:    hash,
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
	http.HandleFunc("/i/", imageHandler)
	http.HandleFunc("/w/", workspaceHandler)
	http.HandleFunc("/recent/", recentHandler)
	http.HandleFunc("/iframe/", iframeHandler)
	http.HandleFunc("/css/", cssHandler)
	http.HandleFunc("/js/", jsHandler)
	http.HandleFunc("/", mainHandler)
	log.Fatal(http.ListenAndServe(*port, nil))
}
