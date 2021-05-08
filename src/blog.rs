use chrono::prelude::*;
use std::{
    fs::{create_dir_all, File},
    io::{stdin, stdout, BufRead, BufReader, Error, ErrorKind, Result, Write},
    path::Path,
};
use walkdir::WalkDir;

// TODO: Differ between markdown and HTML file
pub struct Entry {
    title: String,
    date: Date<Local>,
    path: String,
}

impl Entry {
    pub fn new(title: &str, date_str: &str, path: &str) -> Result<Entry> {
        match &format!("{}T00:00:00Z", date_str).parse::<DateTime<Local>>() {
            Ok(dt) => Ok(Entry {
                title: String::from(title),
                date: Local.ymd(dt.year(), dt.month(), dt.day()),
                path: String::from(path),
            }),
            Err(why) => Err(Error::new(ErrorKind::Other, format!("{}", why))),
        }
    }

    pub fn to_list_entry(&self) -> String {
        format!(
            "{} - {} | {}",
            self.date.format("%Y-%m-%d"),
            self.title,
            self.path
        )
    }

    #[allow(dead_code)]
    pub fn to_html_li(&self, omit: &str) -> String {
        format!(
            "<li>{} - <a href=\"{}\">{}</a></li>\n",
            self.date.format("%Y-%m-%d"),
            &self.path[omit.len()..],
            self.title
        )
    }

    pub fn to_md_li(&self, omit: &str) -> String {
        format!(
            "* {} - [{}]({})\n",
            self.date.format("%Y-%m-%d"),
            self.title,
            &self.path[omit.len()..(self.path.len() - "index.md".len())]
        )
    }
}

pub fn get_list(path: &str) -> Result<Vec<Entry>> {
    let mut entries: Vec<Entry> = vec![];
    for entry in WalkDir::new(&path).min_depth(2).follow_links(true) {
        let entry = entry?.clone();
        if !entry.metadata()?.is_dir() {
            if let Some(ext) = entry.path().extension() {
                match ext.to_str().unwrap() {
                    "md" => {
                        let file = File::open(entry.path().to_str().unwrap())?;
                        let mut reader = BufReader::new(file);
                        let mut title = String::new();
                        let mut date = String::new();

                        reader.read_line(&mut title)?;
                        reader.read_line(&mut date)?;

                        // Remove newline
                        title.pop();
                        date.pop();

                        // TODO: Figure out if H1 header not properly set
                        let title = &title[2..];

                        entries.push(Entry::new(&title, &date, &entry.path().to_str().unwrap())?);
                    }
                    _ => (),
                }
            }
        }
    }
    Ok(entries)
}

fn get_text(label: &str, value: &mut String) -> Result<()> {
    print!("{}: ", label);
    let _ = stdout().flush();

    stdin().read_line(value)?;
    value.pop();
    Ok(())
}

fn get_text_new(label: &str) -> Result<String> {
    let mut value = String::new();
    get_text(label, &mut value)?;
    Ok(value)
}

pub fn new(path: &str) -> Result<()> {
    let name = get_text_new("Name")?;
    let mut date = get_text_new("Date (YYYY-MM-DD | 'now') [Default: 'now']")?;
    if date == "now" || date == "" {
        date = Local::now().format("%Y-%m-%d").to_string();
    }

    match &format!("{}T00:00:00Z", &date).parse::<DateTime<Local>>() {
        Ok(dt) => {
            let date_dir = &dt.format("%Y/%m/%d").to_string();
            let date_fmt = &dt.format("%Y-%m-%d").to_string();
            let mut new_dir = String::from(path);
            if new_dir.chars().nth(new_dir.len() - 1) != Some('/') {
                new_dir += "/";
            }
            new_dir += &date_dir;
            new_dir += "-";
            new_dir += &name.replace(" ", "-").to_lowercase();

            if Path::new(&new_dir).exists() {
                return Err(Error::new(ErrorKind::AlreadyExists, "Entry already exists"));
            }

            create_dir_all(&new_dir)?;

            let mut file = File::create(&format!("{}/index.md", &new_dir))?;
            file.write(&format!("# {}\n{}\n\n", &name, &date_fmt).into_bytes())?;

            println!("'{}' - {} Created", &name, &new_dir);
        }
        Err(why) => {
            eprintln!("ERROR: {}", why);
        }
    }
    Ok(())
}
