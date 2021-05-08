use pulldown_cmark::{html, Options, Parser};
use std::io::Result;

pub fn convert_str_to_html(rstr: &str) -> Result<String> {
    let mut options = Options::empty();
    options.insert(Options::ENABLE_STRIKETHROUGH);
    options.insert(Options::ENABLE_TABLES);
    options.insert(Options::ENABLE_FOOTNOTES);
    options.insert(Options::ENABLE_TASKLISTS);

    let parser = Parser::new_ext(rstr, options);

    let mut html_output = String::new();
    html::push_html(&mut html_output, parser);

    Ok(html_output)
}
