/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
Compile with:

    "...../csc" \
        /lib:"....." \
        /reference:"ReachFramework.dll" \
        /reference:"WindowsBase.dll" \
        /reference:"PresentationCore.dll" \
        /reference:"PresentationFramework.dll" \
        xps_to_png.cs

*/
// logic inspired by this example: https://goo.gl/nCxrjQ
class Program {
    static int ceil(double x) { return (int)System.Math.Ceiling(x); }
    static void convert(double dpi, string path, string out_path) {
        double scale = dpi / 96.0;
        System.Windows.Xps.Packaging.XpsDocument xpsDoc =
                new System.Windows.Xps.Packaging.XpsDocument(
                        path, System.IO.FileAccess.Read);
        if (xpsDoc == null) {
            throw new System.Exception("XpsDocumentfailed");
        }
        System.Windows.Documents.FixedDocumentSequence docSeq =
                xpsDoc.GetFixedDocumentSequence();
        if (docSeq == null) {
            throw new System.Exception("GetFixedDocumentSequence failed");
        }
        System.Windows.Documents.DocumentReferenceCollection drc = docSeq.References;
        int index = 0;
        foreach (System.Windows.Documents.DocumentReference dr in drc) {
            System.Windows.Documents.FixedDocument dp = dr.GetDocument(false);
            foreach (System.Windows.Documents.PageContent pc in dp.Pages) {
                System.Windows.Documents.FixedPage fixedPage = pc.GetPageRoot(false);
                double width = fixedPage.Width;
                double height = fixedPage.Height;
                System.Windows.Size sz = new System.Windows.Size(width, height);
                fixedPage.Measure(sz);
                fixedPage.Arrange(
                        new System.Windows.Rect(new System.Windows.Point(), sz));
                fixedPage.UpdateLayout();
                System.Windows.Media.Imaging.BitmapImage bitmap =
                        new System.Windows.Media.Imaging.BitmapImage();
                System.Windows.Media.Imaging.RenderTargetBitmap renderTarget =
                        new System.Windows.Media.Imaging.RenderTargetBitmap(
                            ceil(scale * width), ceil(scale * height), dpi, dpi,
                            System.Windows.Media.PixelFormats.Default);
                renderTarget.Render(fixedPage);
                System.Windows.Media.Imaging.BitmapEncoder encoder =
                    new System.Windows.Media.Imaging.PngBitmapEncoder();
                encoder.Frames.Add(
                        System.Windows.Media.Imaging.BitmapFrame.Create(renderTarget));
                string filename = string.Format("{0}_{1}.png", out_path, index);
                System.IO.FileStream pageOutStream = new System.IO.FileStream(
                    filename, System.IO.FileMode.Create, System.IO.FileAccess.Write);
                encoder.Save(pageOutStream);
                pageOutStream.Close();
                System.Console.WriteLine(filename);
                ++index;
            }
        }
    }
    // Executes convert, catching thrown exceptions, and printing them
    // to stdout, and exiting immediately.
    static void try_convert(double dpi, string path, string out_path) {
        try {
            convert(dpi, path, out_path);
        } catch (System.Exception e) {
            System.Console.WriteLine(e);
            System.Environment.Exit(1);
        }
    }
    // For each command line argument, convert xps to sequence of pngs.
    static void Main(string[] args) {
        double dpi = 72.0;
        if (args.Length == 0) {
            System.Console.WriteLine("usage:\n\txps_to_png [-dDPI] [XPS_FILES]\n\n");
            System.Environment.Exit(1);
        }
        System.Collections.Generic.List<string> xpsFiles =
                new System.Collections.Generic.List<string>();
        foreach (string arg in args) {
            string flag = "-d";
            if (arg.StartsWith(flag)) {
                dpi = System.Convert.ToDouble(arg.Remove(0, flag.Length));
            } else if (System.IO.File.Exists(arg)) {
                xpsFiles.Add(arg);
            } else {
                System.Console.WriteLine("file missing: '" + arg + "'\n\n");
                System.Environment.Exit(1);
            }
        }
        foreach (string file in xpsFiles) {
            System.Threading.Thread t = new System.Threading.Thread(
                    () => try_convert(dpi, file, file));
            t.SetApartmentState(System.Threading.ApartmentState.STA);
            t.Start();
        }
    }
}
