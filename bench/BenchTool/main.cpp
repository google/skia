//#include <iostream>
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkString.h"

#include "SkBenchmark.h"

typedef SkTRegistry<SkBenchmark> BenchRegistry;

class Iter {
public:
    Iter() {
        fBench = BenchRegistry::Head();
    }
    
    SkBenchmark* next() {
        if (fBench) {
            BenchRegistry::Factory f = fBench->factory();
            fBench = fBench->next();
            return f();
        }
        return NULL;
    }
    
private:
    const BenchRegistry* fBench;
};

static void make_filename(const char name[], SkString* path) {
    path->set(name);
    for (int i = 0; name[i]; i++) {
        switch (name[i]) {
            case '/':
            case '\\':
            case ' ':
            case ':':
                path->writable_str()[i] = '-';
                break;
            default:
                break;
        }
    }
}

int main (int argc, char * const argv[]) {
    const int w = 640;
    const int h = 480;

    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bm.allocPixels();
    
    SkCanvas canvas(bm);
    
    Iter iter;
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        canvas.drawColor(SK_ColorWHITE);
        bench->draw(&canvas);
        
        SkString str;
        make_filename(bench->getName(), &str);
        str.prepend("/skimages/");
        str.append(".png");
        ::remove(str.c_str());
        SkImageEncoder::EncodeFile(str.c_str(), bm, SkImageEncoder::kPNG_Type);
    }
    
    return 0;
}
