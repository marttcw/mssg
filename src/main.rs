use yansi::Paint;

mod core;

#[macro_use]
extern crate clap;

pub fn err_handle(msg: &'static str) {
    eprintln!("{}: {}", Paint::red("ERROR").bold(), msg);
}

fn main() {
    let matches = clap_app!(mssg =>
        (version: crate_version!())
        (author: crate_authors!())
        (about: "Mart's Static Site Generator written in Rust (v2018)")
        (@subcommand init =>
            (about: "Initialize a new site project")
            (@arg PROJ_NAME: "Name of the project")
        )
        (@subcommand gen =>
            (about: "Generate the static site")
        )
        (@subcommand blog =>
            (about: "Manage your blog")
        )
        (@subcommand host =>
            (about: "Host your site")
        )
        (@setting ArgRequiredElseHelp)
    ).get_matches();

    if let Some(matches) = matches.subcommand_matches("init") {
        match matches.is_present("PROJ_NAME") {
            true => core::new::init(matches.value_of("PROJ_NAME").unwrap()).unwrap(),
            false => core::new::init("").unwrap(),
        }
    } else if let Some(_matches) = matches.subcommand_matches("gen") {
        core::gen::gen().unwrap();
    } else if let Some(_matches) = matches.subcommand_matches("blog") {
        /*
            "post" => err_handle("Not supported at the moment", false),
            "edit" => err_handle("Not supported at the moment", false),
            "list" => err_handle("Not supported at the moment", false),
            "delete" => err_handle("Not supported at the moment", false),
        */
    }
}
