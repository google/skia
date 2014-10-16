package main

import (
	"bytes"
	"crypto/md5"
	"database/sql"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"flag"
	"fmt"
	htemplate "html/template"
	"image"
	_ "image/gif"
	_ "image/jpeg"
	"image/png"
	"io/ioutil"
	"math/rand"
	"net"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"text/template"
	"time"
)

import (
	"github.com/fiorix/go-web/autogzip"
	_ "github.com/go-sql-driver/mysql"
	"github.com/golang/glog"
	_ "github.com/mattn/go-sqlite3"
	"github.com/rcrowley/go-metrics"
)

const (
	DEFAULT_SAMPLE = `void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);

    canvas->drawLine(20, 20, 100, 100, p);
}`
	// Don't increase above 2^16 w/o altering the db tables to accept something bigger than TEXT.
	MAX_TRY_SIZE = 64000
)

var (
	// codeTemplate is the cpp code template the user's code is copied into.
	codeTemplate *template.Template = nil

	// gypTemplate is the GYP file to build the executable containing the user's code.
	gypTemplate *template.Template = nil

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
	imageLink = regexp.MustCompile("^/i/([a-z0-9-]+.png)$")

	// tryInfoLink is the regex that matches URLs paths that are direct links to data about a single try.
	tryInfoLink = regexp.MustCompile("^/json/([a-f0-9]+)$")

	// workspaceLink is the regex that matches URLs paths for workspaces.
	workspaceLink = regexp.MustCompile("^/w/([a-z0-9-]+)$")

	// errorRE is ther regex that matches compiler errors and extracts the line / column information.
	errorRE = regexp.MustCompile("^.*.cpp:(\\d+):(\\d+):\\s*(.*)")

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

	gitHash = ""
	gitInfo = ""

	requestsCounter = metrics.NewRegisteredCounter("requests", metrics.DefaultRegistry)
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
	cwd, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		glog.Fatal(err)
	}
	if err := os.Chdir(cwd); err != nil {
		glog.Fatal(err)
	}

	codeTemplate = template.Must(template.ParseFiles(filepath.Join(cwd, "templates/template.cpp")))
	gypTemplate = template.Must(template.ParseFiles(filepath.Join(cwd, "templates/template.gyp")))
	indexTemplate = htemplate.Must(htemplate.ParseFiles(
		filepath.Join(cwd, "templates/index.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
		filepath.Join(cwd, "templates/sidebar.html"),
		filepath.Join(cwd, "templates/content.html"),
		filepath.Join(cwd, "templates/headercommon.html"),
		filepath.Join(cwd, "templates/footercommon.html"),
	))
	iframeTemplate = htemplate.Must(htemplate.ParseFiles(
		filepath.Join(cwd, "templates/iframe.html"),
		filepath.Join(cwd, "templates/content.html"),
		filepath.Join(cwd, "templates/headercommon.html"),
		filepath.Join(cwd, "templates/footercommon.html"),
	))
	recentTemplate = htemplate.Must(htemplate.ParseFiles(
		filepath.Join(cwd, "templates/recent.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
		filepath.Join(cwd, "templates/sidebar.html"),
		filepath.Join(cwd, "templates/headercommon.html"),
		filepath.Join(cwd, "templates/footercommon.html"),
	))
	workspaceTemplate = htemplate.Must(htemplate.ParseFiles(
		filepath.Join(cwd, "templates/workspace.html"),
		filepath.Join(cwd, "templates/titlebar.html"),
		filepath.Join(cwd, "templates/sidebar.html"),
		filepath.Join(cwd, "templates/content.html"),
		filepath.Join(cwd, "templates/headercommon.html"),
		filepath.Join(cwd, "templates/footercommon.html"),
	))

	// The git command returns output of the format:
	//
	//   f672cead70404080a991ebfb86c38316a4589b23 2014-04-27 19:21:51 +0000
	//
	logOutput, err := doCmd(`git log --format=%H%x20%ai HEAD^..HEAD`)
	if err != nil {
		panic(err)
	}
	logInfo := strings.Split(logOutput, " ")
	gitHash = logInfo[0]
	gitInfo = logInfo[1] + " " + logInfo[2] + " " + logInfo[0][0:6]

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
			glog.Errorf("Failed to read password from metadata server: %q\n", err)
			panic(err)
		}
		// The IP address of the database is found here:
		//    https://console.developers.google.com/project/31977622648/sql/instances/webtry/overview
		// And 3306 is the default port for MySQL.
		db, err = sql.Open("mysql", fmt.Sprintf("webtry:%s@tcp(173.194.83.52:3306)/webtry?parseTime=true", password))
		if err != nil {
			glog.Errorf("ERROR: Failed to open connection to SQL server: %q\n", err)
			panic(err)
		}
	} else {
		glog.Infof("Failed to find metadata, unable to connect to MySQL server (Expected when running locally): %q\n", err)
		// Fallback to sqlite for local use.
		db, err = sql.Open("sqlite3", "./webtry.db")
		if err != nil {
			glog.Errorf("Failed to open: %q\n", err)
			panic(err)
		}
		sql := `CREATE TABLE IF NOT EXISTS source_images (
             id        INTEGER     PRIMARY KEY                NOT NULL,
             image     MEDIUMBLOB  DEFAULT ''                 NOT NULL, -- formatted as a PNG.
             width     INTEGER     DEFAULT 0                  NOT NULL,
             height    INTEGER     DEFAULT 0                  NOT NULL,
             create_ts TIMESTAMP   DEFAULT CURRENT_TIMESTAMP  NOT NULL,
             hidden    INTEGER     DEFAULT 0                  NOT NULL
             )`
		_, err = db.Exec(sql)
		if err != nil {
			glog.Infof("status creating sqlite table for sources: %q\n", err)
		}

		sql = `CREATE TABLE IF NOT EXISTS webtry (
             code               TEXT      DEFAULT ''                 NOT NULL,
             create_ts          TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
             hash               CHAR(64)  DEFAULT ''                 NOT NULL,
             width              INTEGER   DEFAULT 256                NOT NULL,
             height             INTEGER   DEFAULT 256                NOT NULL,
             gpu                BOOL      DEFAULT 0                  NOT NULL,
             source_image_id    INTEGER   DEFAULT 0                  NOT NULL,

             PRIMARY KEY(hash)
            )`
		_, err = db.Exec(sql)
		if err != nil {
			glog.Infof("status creating sqlite table for webtry: %q\n", err)
		}

		sql = `CREATE TABLE IF NOT EXISTS workspace (
          name      CHAR(64)  DEFAULT ''                 NOT NULL,
          create_ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
          PRIMARY KEY(name)
        )`
		_, err = db.Exec(sql)
		if err != nil {
			glog.Infof("status creating sqlite table for workspace: %q\n", err)
		}

		sql = `CREATE TABLE IF NOT EXISTS workspacetry (
          name               CHAR(64)  DEFAULT ''                 NOT NULL,
          create_ts          TIMESTAMP DEFAULT CURRENT_TIMESTAMP  NOT NULL,
          hash               CHAR(64)  DEFAULT ''                 NOT NULL,
          width              INTEGER   DEFAULT 256                NOT NULL,
          height             INTEGER   DEFAULT 256                NOT NULL,
          gpu                BOOL      DEFAULT 0                  NOT NULL,
          hidden             INTEGER   DEFAULT 0                  NOT NULL,
          source_image_id    INTEGER   DEFAULT 0                  NOT NULL,

          FOREIGN KEY (name)   REFERENCES workspace(name)
        )`
		_, err = db.Exec(sql)
		if err != nil {
			glog.Infof("status creating sqlite table for workspace try: %q\n", err)
		}
	}

	// Ping the database to keep the connection fresh.
	go func() {
		c := time.Tick(1 * time.Minute)
		for _ = range c {
			if err := db.Ping(); err != nil {
				glog.Errorf("Database failed to respond: %q\n", err)
			}
		}
	}()

	metrics.RegisterRuntimeMemStats(metrics.DefaultRegistry)
	go metrics.CaptureRuntimeMemStats(metrics.DefaultRegistry, 1*time.Minute)

	// Start reporting metrics.
	// TODO(jcgregorio) We need a centrialized config server for storing things
	// like the IP address of the Graphite monitor.
	addr, _ := net.ResolveTCPAddr("tcp", "skia-monitoring-b:2003")
	go metrics.Graphite(metrics.DefaultRegistry, 1*time.Minute, "webtry", addr)

	writeOutAllSourceImages()
}

func writeOutAllSourceImages() {
	// Pull all the source images from the db and write them out to inout.
	rows, err := db.Query("SELECT id, image, create_ts FROM source_images ORDER BY create_ts DESC")
	if err != nil {
		glog.Errorf("Failed to open connection to SQL server: %q\n", err)
		panic(err)
	}
	defer rows.Close()
	for rows.Next() {
		var id int
		var image []byte
		var create_ts time.Time
		if err := rows.Scan(&id, &image, &create_ts); err != nil {
			glog.Errorf("failed to fetch from database: %q", err)
			continue
		}
		filename := fmt.Sprintf("../../../inout/image-%d.png", id)
		if _, err := os.Stat(filename); os.IsExist(err) {
			glog.Infof("Skipping write since file exists: %q", filename)
			continue
		}
		if err := ioutil.WriteFile(filename, image, 0666); err != nil {
			glog.Errorf("failed to write image file: %q", err)
		}
	}
}

// Titlebar is used in titlebar template expansion.
type Titlebar struct {
	GitHash string
	GitInfo string
}

// userCode is used in template expansion.
type userCode struct {
	Code     string
	Hash     string
	Width    int
	Height   int
	GPU      bool
	Source   int
	Titlebar Titlebar
}

// writeTemplate creates a given output file and writes the template
// result there.
func writeTemplate(filename string, t *template.Template, context interface{}) error {
	f, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer f.Close()
	return t.Execute(f, context)
}

// expandToFile expands the template and writes the result to the file.
func expandToFile(filename string, code string, t *template.Template) error {
	return writeTemplate(filename, t, userCode{
		Code:     code,
		Titlebar: Titlebar{GitHash: gitHash, GitInfo: gitInfo},
	})
}

// expandCode expands the template into a file and calculates the MD5 hash.
// We include the width and height here so that a single hash can capture
// both the code and the supplied width/height parameters.
func expandCode(code string, source int, width, height int, gpu bool) (string, error) {
	// in order to support fonts in the chroot jail, we need to make sure
	// we're using portable typefaces.
	// TODO(humper):  Make this more robust, supporting things like setTypeface

	inputCodeLines := strings.Split(code, "\n")
	outputCodeLines := []string{
		"DECLARE_bool(portableFonts);",
		fmt.Sprintf("// WxH: %d, %d", width, height),
		fmt.Sprintf("// GPU: %v", gpu),
	}
	for _, line := range inputCodeLines {
		outputCodeLines = append(outputCodeLines, line)
		if strings.HasPrefix(strings.TrimSpace(line), "SkPaint p") {
			outputCodeLines = append(outputCodeLines, "FLAGS_portableFonts = true;")
			outputCodeLines = append(outputCodeLines, "sk_tool_utils::set_portable_typeface(&p, \"Helvetica\", SkTypeface::kNormal);")
		}
	}

	fontFriendlyCode := strings.Join(outputCodeLines, "\n")

	h := md5.New()
	h.Write([]byte(fontFriendlyCode))
	binary.Write(h, binary.LittleEndian, int64(source))
	hash := fmt.Sprintf("%x", h.Sum(nil))
	// At this point we are running in skia/experimental/webtry, making cache a
	// peer directory to skia.
	// TODO(jcgregorio) Make all relative directories into flags.
	err := expandToFile(fmt.Sprintf("../../../cache/src/%s.cpp", hash), fontFriendlyCode, codeTemplate)
	return hash, err
}

// expandGyp produces the GYP file needed to build the code
func expandGyp(hash string) error {
	return writeTemplate(fmt.Sprintf("../../../cache/%s.gyp", hash), gypTemplate, struct{ Hash string }{hash})
}

// response is serialized to JSON as a response to POSTs.
type response struct {
	Message       string         `json:"message"`
	CompileErrors []compileError `json:"compileErrors"`
	Img           string         `json:"img"`
	Hash          string         `json:"hash"`
}

// doCmd executes the given command line string; the command being
// run is expected to not care what its current working directory is.
// Returns the stdout and stderr.
func doCmd(commandLine string) (string, error) {
	glog.Infof("Command: %q\n", commandLine)
	programAndArgs := strings.SplitN(commandLine, " ", 2)
	program := programAndArgs[0]
	args := []string{}
	if len(programAndArgs) > 1 {
		args = strings.Split(programAndArgs[1], " ")
	}
	cmd := exec.Command(program, args...)
	message, err := cmd.CombinedOutput()
	glog.Infof("StdOut + StdErr: %s\n", string(message))
	if err != nil {
		glog.Errorf("Exit status: %s\n", err)
		return string(message), fmt.Errorf("Failed to run command.")
	}
	return string(message), nil
}

// reportError formats an HTTP error response and also logs the detailed error message.
func reportError(w http.ResponseWriter, r *http.Request, err error, message string) {
	glog.Errorf("%s\n%s", message, err)
	w.Header().Set("Content-Type", "text/plain")
	http.Error(w, message, 500)
}

// reportTryError formats an HTTP error response in JSON and also logs the detailed error message.
func reportTryError(w http.ResponseWriter, r *http.Request, err error, message, hash string) {
	m := response{
		Message: message,
		Hash:    hash,
	}
	glog.Errorf("%s\n%s", message, err)
	resp, err := json.Marshal(m)

	if err != nil {
		http.Error(w, "Failed to serialize a response", 500)
		return
	}
	w.Header().Set("Content-Type", "text/plain")
	w.Write(resp)
}

func reportCompileError(w http.ResponseWriter, r *http.Request, compileErrors []compileError, hash string) {
	m := response{
		CompileErrors: compileErrors,
		Hash:          hash,
	}

	resp, err := json.Marshal(m)

	if err != nil {
		http.Error(w, "Failed to serialize a response", 500)
		return
	}
	w.Header().Set("Content-Type", "text/plain")
	w.Write(resp)
}

func writeToDatabase(hash string, code string, workspaceName string, source int, width, height int, gpu bool) {
	if db == nil {
		return
	}
	if _, err := db.Exec("INSERT INTO webtry (code, hash, width, height, gpu, source_image_id) VALUES(?, ?, ?, ?, ?, ?)", code, hash, width, height, gpu, source); err != nil {
		glog.Errorf("Failed to insert code into database: %q\n", err)
	}
	if workspaceName != "" {
		if _, err := db.Exec("INSERT INTO workspacetry (name, hash, width, height, gpu, source_image_id) VALUES(?, ?, ?, ?, ?, ?)", workspaceName, hash, width, height, gpu, source); err != nil {
			glog.Errorf("Failed to insert into workspacetry table: %q\n", err)
		}
	}
}

type Sources struct {
	Id int `json:"id"`
}

// sourcesHandler serves up the PNG of a specific try.
func sourcesHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Sources Handler: %q\n", r.URL.Path)
	if r.Method == "GET" {
		rows, err := db.Query("SELECT id, create_ts FROM source_images WHERE hidden=0 ORDER BY create_ts DESC")
		if err != nil {
			http.Error(w, fmt.Sprintf("Failed to query sources: %s.", err), 500)
		}
		defer rows.Close()
		sources := make([]Sources, 0, 0)
		for rows.Next() {
			var id int
			var create_ts time.Time
			if err := rows.Scan(&id, &create_ts); err != nil {
				glog.Errorf("failed to fetch from database: %q", err)
				continue
			}
			sources = append(sources, Sources{Id: id})
		}

		resp, err := json.Marshal(sources)
		if err != nil {
			reportError(w, r, err, "Failed to serialize a response.")
			return
		}
		w.Header().Set("Content-Type", "application/json")
		w.Write(resp)

	} else if r.Method == "POST" {
		if err := r.ParseMultipartForm(1000000); err != nil {
			http.Error(w, fmt.Sprintf("Failed to load image: %s.", err), 500)
			return
		}
		if _, ok := r.MultipartForm.File["upload"]; !ok {
			http.Error(w, "Invalid upload.", 500)
			return
		}
		if len(r.MultipartForm.File["upload"]) != 1 {
			http.Error(w, "Wrong number of uploads.", 500)
			return
		}
		f, err := r.MultipartForm.File["upload"][0].Open()
		if err != nil {
			http.Error(w, fmt.Sprintf("Failed to load image: %s.", err), 500)
			return
		}
		defer f.Close()
		m, _, err := image.Decode(f)
		if err != nil {
			http.Error(w, fmt.Sprintf("Failed to decode image: %s.", err), 500)
			return
		}
		var b bytes.Buffer
		png.Encode(&b, m)
		bounds := m.Bounds()
		width := bounds.Max.Y - bounds.Min.Y
		height := bounds.Max.X - bounds.Min.X
		if _, err := db.Exec("INSERT INTO source_images (image, width, height) VALUES(?, ?, ?)", b.Bytes(), width, height); err != nil {
			glog.Errorf("Failed to insert sources into database: %q\n", err)
			http.Error(w, fmt.Sprintf("Failed to store image: %s.", err), 500)
			return
		}
		go writeOutAllSourceImages()

		// Now redirect back to where we came from.
		http.Redirect(w, r, r.Referer(), 302)
	} else {
		http.NotFound(w, r)
		return
	}
}

// imageHandler serves up the PNG of a specific try.
func imageHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Image Handler: %q\n", r.URL.Path)
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
	w.Header().Set("Content-Type", "image/png")
	http.ServeFile(w, r, fmt.Sprintf("../../../inout/%s", filename))
}

type Try struct {
	Hash     string `json:"hash"`
	Source   int
	CreateTS string `json:"create_ts"`
}

type Recent struct {
	Tries    []Try
	Titlebar Titlebar
}

// recentHandler shows the last 20 tries.
func recentHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Recent Handler: %q\n", r.URL.Path)

	rows, err := db.Query("SELECT create_ts, hash FROM webtry ORDER BY create_ts DESC LIMIT 20")
	if err != nil {
		http.NotFound(w, r)
		return
	}
	defer rows.Close()
	recent := []Try{}
	for rows.Next() {
		var hash string
		var create_ts time.Time
		if err := rows.Scan(&create_ts, &hash); err != nil {
			glog.Errorf("failed to fetch from database: %q", err)
			continue
		}
		recent = append(recent, Try{Hash: hash, CreateTS: create_ts.Format("2006-02-01")})
	}
	w.Header().Set("Content-Type", "text/html")
	if err := recentTemplate.Execute(w, Recent{Tries: recent, Titlebar: Titlebar{GitHash: gitHash, GitInfo: gitInfo}}); err != nil {
		glog.Errorf("Failed to expand template: %q\n", err)
	}
}

type Workspace struct {
	Name     string
	Code     string
	Hash     string
	Width    int
	Height   int
	Source   int
	GPU      bool
	Tries    []Try
	Titlebar Titlebar
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
			glog.Errorf("Failed to insert workspace into database: %q\n", err)
		}
	}
	return "", fmt.Errorf("Failed to create a new workspace")
}

// getCode returns the code for a given hash, or the empty string if not found.
func getCode(hash string) (string, int, int, int, bool, error) {
	code := ""
	width := 0
	height := 0
	source := 0
	gpu := false
	if err := db.QueryRow("SELECT code, width, height, gpu, source_image_id FROM webtry WHERE hash=?", hash).Scan(&code, &width, &height, &gpu, &source); err != nil {
		glog.Errorf("Code for hash is missing: %q\n", err)
		return code, width, height, source, gpu, err
	}
	return code, width, height, source, gpu, nil
}

func workspaceHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Workspace Handler: %q\n", r.URL.Path)
	if r.Method == "GET" {
		tries := []Try{}
		match := workspaceLink.FindStringSubmatch(r.URL.Path)
		name := ""
		if len(match) == 2 {
			name = match[1]
			rows, err := db.Query("SELECT create_ts, hash, source_image_id FROM workspacetry WHERE name=? ORDER BY create_ts", name)
			if err != nil {
				reportError(w, r, err, "Failed to select.")
				return
			}
			defer rows.Close()
			for rows.Next() {
				var hash string
				var create_ts time.Time
				var source int
				if err := rows.Scan(&create_ts, &hash, &source); err != nil {
					glog.Errorf("failed to fetch from database: %q", err)
					continue
				}
				tries = append(tries, Try{Hash: hash, Source: source, CreateTS: create_ts.Format("2006-02-01")})
			}
		}
		var code string
		var hash string
		var width int
		var height int
		source := 0
		gpu := false
		if len(tries) == 0 {
			code = DEFAULT_SAMPLE
			width = 256
			height = 256
		} else {
			hash = tries[len(tries)-1].Hash
			code, width, height, source, gpu, _ = getCode(hash)
		}
		w.Header().Set("Content-Type", "text/html")
		if err := workspaceTemplate.Execute(w, Workspace{Tries: tries, Code: code, Name: name, Hash: hash, Width: width, Height: height, GPU: gpu, Source: source, Titlebar: Titlebar{GitHash: gitHash, GitInfo: gitInfo}}); err != nil {
			glog.Errorf("Failed to expand template: %q\n", err)
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
	Code   string `json:"code"`
	Width  int    `json:"width"`
	Height int    `json:"height"`
	GPU    bool   `json:"gpu"`
	Name   string `json:"name"`   // Optional name of the workspace the code is in.
	Source int    `json:"source"` // ID of the source image, 0 if none.
}

// iframeHandler handles the GET and POST of the main page.
func iframeHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("IFrame Handler: %q\n", r.URL.Path)
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
	code, width, height, source, gpu, err := getCode(hash)
	if err != nil {
		http.NotFound(w, r)
		return
	}
	// Expand the template.
	w.Header().Set("Content-Type", "text/html")
	if err := iframeTemplate.Execute(w, userCode{Code: code, Width: width, Height: height, GPU: gpu, Hash: hash, Source: source}); err != nil {
		glog.Errorf("Failed to expand template: %q\n", err)
	}
}

type TryInfo struct {
	Hash   string `json:"hash"`
	Code   string `json:"code"`
	Width  int    `json:"width"`
	Height int    `json:"height"`
	GPU    bool   `json:"gpu"`
	Source int    `json:"source"`
}

// tryInfoHandler returns information about a specific try.
func tryInfoHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Try Info Handler: %q\n", r.URL.Path)
	if r.Method != "GET" {
		http.NotFound(w, r)
		return
	}
	match := tryInfoLink.FindStringSubmatch(r.URL.Path)
	if len(match) != 2 {
		http.NotFound(w, r)
		return
	}
	hash := match[1]
	code, width, height, source, gpu, err := getCode(hash)
	if err != nil {
		http.NotFound(w, r)
		return
	}
	m := TryInfo{
		Hash:   hash,
		Code:   code,
		Width:  width,
		Height: height,
		GPU:    gpu,
		Source: source,
	}
	resp, err := json.Marshal(m)
	if err != nil {
		reportError(w, r, err, "Failed to serialize a response.")
		return
	}
	w.Header().Set("Content-Type", "application/json")
	w.Write(resp)
}

func cleanCompileOutput(s, hash string) string {
	old := "../../../cache/src/" + hash + ".cpp:"
	glog.Infof("replacing %q\n", old)
	return strings.Replace(s, old, "usercode.cpp:", -1)
}

type compileError struct {
	Line   int    `json:"line"`
	Column int    `json:"column"`
	Error  string `json:"error"`
}

// mainHandler handles the GET and POST of the main page.
func mainHandler(w http.ResponseWriter, r *http.Request) {
	glog.Infof("Main Handler: %q\n", r.URL.Path)
	requestsCounter.Inc(1)
	if r.Method == "GET" {
		code := DEFAULT_SAMPLE
		source := 0
		width := 256
		height := 256
		gpu := false
		match := directLink.FindStringSubmatch(r.URL.Path)
		var hash string
		if len(match) == 2 && r.URL.Path != "/" {
			hash = match[1]
			if db == nil {
				http.NotFound(w, r)
				return
			}
			// Update 'code' with the code found in the database.
			if err := db.QueryRow("SELECT code, width, height, gpu, source_image_id FROM webtry WHERE hash=?", hash).Scan(&code, &width, &height, &gpu, &source); err != nil {
				http.NotFound(w, r)
				return
			}
		}
		// Expand the template.
		w.Header().Set("Content-Type", "text/html")
		if err := indexTemplate.Execute(w, userCode{Code: code, Hash: hash, Source: source, Width: width, Height: height, GPU: gpu, Titlebar: Titlebar{GitHash: gitHash, GitInfo: gitInfo}}); err != nil {
			glog.Errorf("Failed to expand template: %q\n", err)
		}
	} else if r.Method == "POST" {
		w.Header().Set("Content-Type", "application/json")
		buf := bytes.NewBuffer(make([]byte, 0, MAX_TRY_SIZE))
		n, err := buf.ReadFrom(r.Body)
		if err != nil {
			reportTryError(w, r, err, "Failed to read a request body.", "")
			return
		}
		if n == MAX_TRY_SIZE {
			err := fmt.Errorf("Code length equal to, or exceeded, %d", MAX_TRY_SIZE)
			reportTryError(w, r, err, "Code too large.", "")
			return
		}
		request := TryRequest{}
		if err := json.Unmarshal(buf.Bytes(), &request); err != nil {
			reportTryError(w, r, err, "Coulnd't decode JSON.", "")
			return
		}
		if hasPreProcessor(request.Code) {
			err := fmt.Errorf("Found preprocessor macro in code.")
			reportTryError(w, r, err, "Preprocessor macros aren't allowed.", "")
			return
		}
		hash, err := expandCode(LineNumbers(request.Code), request.Source, request.Width, request.Height, request.GPU)
		if err != nil {
			reportTryError(w, r, err, "Failed to write the code to compile.", hash)
			return
		}
		writeToDatabase(hash, request.Code, request.Name, request.Source, request.Width, request.Height, request.GPU)
		err = expandGyp(hash)
		if err != nil {
			reportTryError(w, r, err, "Failed to write the gyp file.", hash)
			return
		}
		cmd := fmt.Sprintf("scripts/fiddle_wrapper %s --width %d --height %d", hash, request.Width, request.Height)
		if request.GPU {
			cmd += " --gpu"
		}
		if *useChroot {
			cmd = "schroot -c webtry --directory=/ -- /skia_build/skia/experimental/webtry/" + cmd
		}
		if request.Source > 0 {
			cmd += fmt.Sprintf(" --source image-%d.png", request.Source)
		}

		message, err := doCmd(cmd)

		outputLines := strings.Split(message, "\n")
		errorLines := []compileError{}
		for _, line := range outputLines {
			match := errorRE.FindStringSubmatch(line)
			if len(match) > 0 {
				lineNumber, parseError := strconv.Atoi(match[1])
				if parseError != nil {
					glog.Errorf("ERROR: Couldn't parse line number from %s\n", match[1])
					continue
				}
				columnNumber, parseError := strconv.Atoi(match[2])
				if parseError != nil {
					glog.Errorf("ERROR: Couldn't parse column number from %s\n", match[2])
					continue
				}
				errorLines = append(errorLines,
					compileError{
						Line:   lineNumber,
						Column: columnNumber,
						Error:  match[3],
					})
			}
		}

		if err != nil {
			if len(errorLines) > 0 {
				reportCompileError(w, r, errorLines, hash)
			} else {
				reportTryError(w, r, err, "Failed to run the code:\n"+message, hash)
			}
			return
		}

		png, err := ioutil.ReadFile("../../../inout/" + hash + ".png")
		if err != nil {
			reportTryError(w, r, err, "Failed to open the generated PNG.", hash)
			return
		}

		m := response{
			Img:  base64.StdEncoding.EncodeToString([]byte(png)),
			Hash: hash,
		}
		resp, err := json.Marshal(m)
		if err != nil {
			reportTryError(w, r, err, "Failed to serialize a response.", hash)
			return
		}
		w.Header().Set("Content-Type", "application/json")
		w.Write(resp)
	}
}

func main() {
	flag.Parse()
	http.HandleFunc("/i/", autogzip.HandleFunc(imageHandler))
	http.HandleFunc("/w/", autogzip.HandleFunc(workspaceHandler))
	http.HandleFunc("/recent/", autogzip.HandleFunc(recentHandler))
	http.HandleFunc("/iframe/", autogzip.HandleFunc(iframeHandler))
	http.HandleFunc("/json/", autogzip.HandleFunc(tryInfoHandler))
	http.HandleFunc("/sources/", autogzip.HandleFunc(sourcesHandler))

	// Resources are served directly
	// TODO add support for caching/etags/gzip
	http.Handle("/res/", autogzip.Handle(http.FileServer(http.Dir("./"))))

	// TODO Break out /c/ as it's own handler.
	http.HandleFunc("/", autogzip.HandleFunc(mainHandler))
	glog.Fatal(http.ListenAndServe(*port, nil))
}
