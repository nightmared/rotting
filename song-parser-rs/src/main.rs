#[macro_use] extern crate lazy_static;
extern crate regex;
extern crate getopts;

use getopts::Options;
use std::env;
use std::fs::{self, File};
use std::io::prelude::*;

mod parser;

use parser::{Song, SongList, Backend};

static BASE_DIR: &'static str = "/home/t0b1nux/documents/chants-fel/chants-fel-tex/originaux/";
static HTML_OUT_FILE: &'static str = "src/html/index.html";
static LATEX_OUT_FILE: &'static str = "out.tex"; 

fn read_file(path: &str) -> String {
    let mut f = File::open(path).unwrap();
    let mut string = String::new();
    f.read_to_string(&mut string).unwrap();
    string
}


fn print_usage(program: &str, opts: Options) {
    let brief = format!("Usage: {} FILE [options]", program);
    print!("{}", opts.usage(&brief));
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let program = args[0].clone();

    let mut opts = Options::new();
    opts.optopt("l", "latex", "export LaTeX file instead of HTML", "");
    opts.optflag("h", "help", "print this help menu");
    let matches = opts.parse(&args[1..]).expect("Couldn't parse arguments !");

    if matches.opt_present("h") {
        print_usage(&program, opts);
        return;
    }

    let mut backend = Backend::HTML;
    let header;
    let footer;
    let mut out_file;

    if matches.opt_present("l") {
        // LaTeX
        backend = Backend::LaTeX;
        header = read_file(&format!("{}{}", BASE_DIR, "../main.tex"));
        footer = "\n\\end{document}".to_owned();
        out_file = File::create(LATEX_OUT_FILE).unwrap();
    } else {
        // HTML
        header = read_file("src/html/header.html");
        footer = read_file("src/html/footer.html");
        out_file = File::create(HTML_OUT_FILE).unwrap();
    }
    

    let mut vec = vec![];
    for entry in fs::read_dir(BASE_DIR).unwrap() {
        let entry = entry.unwrap();

        let mut song_file = File::open(entry.path()).unwrap();
        let mut song_string = String::new();
        song_file.read_to_string(&mut song_string).unwrap();
        let song = Song::new(song_string);
        vec.push(song);
    }
    let mut list = SongList::new(vec);
    
    out_file.write(
        list
        .gen(backend, &header, &footer)
        .expect("Couldn't export the given list !")
        .as_bytes()
    ).expect("Couldn't write to output file !");
}
