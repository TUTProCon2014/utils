import std.stdio;
import std.getopt;
import std.regex;
import std.file;

enum info = `
--to:
    --to=lf    : [CRLF, LF] -> LF
    --to=crlf  : [CRLF, LF] -> CRLF

--dir   : target dirs
    --dir='../foo'
--rec   : recursive ?
    --rec=true, --rec=false
`;

enum Target
{
    lf = "\n",
    crlf = "\r\n",
}


void main(string[] args)
{
    Target tgt;
    string dir = ".";
    bool rec = true;

    if(args.length == 1){
        writeln(info);
        return;
    }


    getopt(args,
        "to", &tgt,
        "dir", &dir,
        "rec", &rec);

    apply(dir, tgt, rec);
}


void apply(string dir, Target convTo, bool recurse)
{
    auto mode = recurse ? SpanMode.breadth : SpanMode.shallow;

    foreach(de; dirEntries(dir, "*.{h,hpp,cpp,c}", mode)){
        if(de.isFile){
            string fname = de.name;
            writeln("convert : ", fname);

            std.file.write(fname,
                readText(fname)
                .replaceAll(ctRegex!`\r?\n`, convTo)
            );
        }
    }
}
