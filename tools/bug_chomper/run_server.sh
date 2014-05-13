if [[ -z `which go` ]]; then
  echo "Please install Go before running the server."
  exit 1
fi

go get github.com/gorilla/securecookie

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd $DIR
GOPATH="$GOPATH:$DIR" go run $DIR/src/server/server.go $@

