use regex::Regex;

pub struct Song {
    title: String,
    chorus: Option<String>,
    verses: Vec<String>
}

pub struct SongList {
    songs: Vec<Song>
}

#[derive(PartialEq)]
pub enum Backend {
    LaTeX,
    HTML
}

enum LineType<'a> {
    Title(&'a str),
    //Attribute { title: &str, value: &str },
    Text(&'a str),
    NoChorus,
    Newline,
    Unknown
}


macro_rules! latex_cmd {
    ($cmd:expr, $val:expr) => {
        &format!("\\{}{{{}}}", $cmd, $val)
    }
}

macro_rules! html_tag {
    ($name:expr, $value:expr, $prop:expr) => (&format!("<{} {}>{}</{}>", $name, $prop, $value, $name));
    ($name:expr, $value:expr) => (html_tag!($name, $value, ""));
}

macro_rules! insert_frame {
    ($out:expr, $title:expr) => {
        $out.push_str("<section>");
        $out.push_str(html_tag!("h2", $title));
    };
    ($out:expr) => (insert_frame!($out, ""));
}

macro_rules! close_frame {
    ($out:expr) => ($out.push_str("</section>"));
}

fn add_verse(out: &mut String, verse: &str, num: usize) {
    out.push_str(latex_cmd!("begin", "frame"));
    if num == 0 {
        out.push_str(latex_cmd!("frametitle", "Refrain"));
        out.push_str(verse);
    } else {
        out.push_str(latex_cmd!("frametitle", format!("Couplet {}", num)));
        out.push_str(verse);
    }
    out.push_str(latex_cmd!("end", "frame"));
}

fn parse_line(line: &str) -> LineType {
    // New parser without regexpes in progress
    lazy_static! {
        static ref TITLE_RE: Regex = Regex::new(r"^#titre=(?P<value>[ÉÈÊÀÙÛÎÔÂÇéèêàùeôâîûçcœ,;\?\.!''’a-zA-Z0-9 \t]+)$").unwrap();
        static ref ATTR_RE: Regex = Regex::new("^#(?P<attr>[:word:]+)$").unwrap();
        static ref TXT_RE: Regex = Regex::new(r"^[ÉÈÊÀÙÛÎÔÂÇéèêàùeôâîûçcœ,;\?\.!'’\(\)a-zA-Z0-9 \t]+$").unwrap();
        static ref NEWLINE_RE: Regex = Regex::new("^\r?\n?$").unwrap();
    }
    if NEWLINE_RE.is_match(line) {
        return LineType::Newline;
    }
    let mut cap = TITLE_RE.captures(line);
    if cap.is_some() {
        return LineType::Title(cap.unwrap().name("value").unwrap());
    }
    
    cap = ATTR_RE.captures(line);
    if cap.is_some() && cap.unwrap().name("attr").unwrap() == "nochorus" {
        return LineType::NoChorus;
    }

    if TXT_RE.is_match(line) {
        return LineType::Text(line);
    }

    LineType::Unknown
}

impl Song {
    pub fn new(file: String) -> Song {
        let mut lines = file.lines();

        let mut title = String::new();
        let mut chorus = true;
        let mut chorus_string = String::new();
        // parse chorus first
        while let Some(x) = lines.next() {
            match parse_line(x) {
                LineType::Title(value) => {
                    title = value.into();
                }
                LineType::NoChorus => {
                    chorus = false;
                    break;
                }
                LineType::Text(value) => {
                    chorus_string.push_str(value);
                    chorus_string.push_str("\n");
                }
                LineType::Newline => {
                    if chorus == false || !chorus_string.is_empty() {
                        break;
                    }
                }
                _ => {}
            }
        };

        let mut verses: Vec<String> = Vec::new();
        verses.push(String::new());
        let mut current_verse = 0;
        while let Some(x) = lines.next() {
            match parse_line(x) {
                LineType::Title(value) => {
                    title = value.into();
                }
                LineType::Newline => {
                    // current verse isn't empty <=> verse had already been parsed
                    if !verses[current_verse].is_empty() {
                        current_verse += 1;
                        verses.push(String::new());
                    }
                }
                LineType::Text(value) => {
                   verses[current_verse].push_str(value);
                }
                _ => {}
            }
        };
        Song { title: title, chorus: Some(chorus_string), verses: verses }
    }

    pub fn gen_tex(&self) -> String {
        // TODO: predict the size to decrease memory allocations
        let mut s = String::new();
        s.push_str(latex_cmd!("section", self.title));
        let mut current_verse = 0;
        for verse in &self.verses {
            current_verse += 1;
            if let Some(chorus) = self.chorus.clone() {
                add_verse(&mut s, &chorus, 0);
            }
            add_verse(&mut s, &verse, current_verse);
        }
        s
    }
   

    pub fn gen_html(&self) -> String {
        // TODO: predict the size to decrease memory allocations
        let mut s = String::new();
        
        // Do not compute the chorus more than once
        let mut chorus = String::from("");
        if self.chorus.is_some() { 
            let mut unwrapped = self.chorus.clone().unwrap();
            insert_frame!(&chorus, format!("{} - Refrain", self.title));
                let splits: Vec<&str> = (&self.chorus).split('\n').collect();
                for line in splits {
                    chorus.push_str(html_tag!("p", line));
                }
                close_frame!(chorus);
            }

        let mut verse_no = 1;
        for verse in &self.verses {
            s.push_str(&chorus);
            insert_frame!(s, format!("{} - Couplet {}", self.title, verse_no));
            let splits: Vec<&str> = verse.split('\n').collect();
            for i in splits {
                s.push_str(html_tag!("p", i));
            }
            verse_no += 1;
            close_frame!(s);
        }
        s.push_str(&chorus);
        s
    }
}

impl SongList {
    pub fn new(vec: Vec<Song>) -> SongList {
        SongList { 
            songs: vec
        }
    }

    pub fn gen(&self, backend: Backend, prelude: &str, conclusion: &str) -> Result<String, String> {
        let mut s = String::new();
        s.push_str(prelude);
        // generate the TOC for HTML, LaTeX does it automatically
        if backend == Backend::HTML {
            let threshold = 8;
            let mut i = 0;
            insert_frame!(s, format!("Sommaire - Page {}/{}", i/threshold, self.songs.len()/threshold));
            for song in &self.songs {
                i += 1;
                if i % threshold == 0 {
                    close_frame!(s);
                    insert_frame!(s, format!("Sommaire - Page {}/{}", i/threshold, self.songs.len()/threshold));
                }
                s.push_str(html_tag!("a", html_tag!("p", format!("{}. {}", i, song.title)), format!("href=\"#/song-{}\"", i)));
            }
            close_frame!(s);
        }

        
        let vec: Vec<String> = self.songs
            .iter()
            .enumerate()
            .map(|(num, song): (usize, &Song)| {
                match backend {
                    Backend::LaTeX => song.gen_tex(),
                    Backend::HTML => format!("{}{}", html_tag!("section", html_tag!("h1", song.title), format!("id=\"song-{}\"", num+1)), song.gen_html())
                }
            }).collect();
    
        let string: String = vec.iter()
            .flat_map(|s| s.chars())
            .collect();

        s.push_str(&string);


        s.push_str(conclusion);
        Ok(s)
    }
}
