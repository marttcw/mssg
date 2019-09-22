use yansi::Paint;
use std::panic::set_hook;

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
        (about: crate_description!())
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
            true => {
                ::std::process::exit(match core::new::init(matches.value_of("PROJ_NAME").unwrap()) {
                    Ok(_) => 0,
                    Err(err) => {
                        eprintln!("{}: Could not initialize: Project \"{}\" already exists.\n\t{}: {:?}"
                            , Paint::red("ERROR").bold(), matches.value_of("PROJ_NAME").unwrap(), Paint::red("ERROR DESCRIPTION").bold(), err);
                        1
                    }
                });
            }
            false => {
                ::std::process::exit(match core::new::init("") {
                    Ok(_) => 0,
                    Err(err) => {
                        eprintln!("{}: Could not initialize: Project already exists.\n\t{}: {:?}"
                            , Paint::red("ERROR").bold(), Paint::red("ERROR DESCRIPTION").bold(), err);
                        1
                    }
                });
            }
        }
    } else if let Some(_matches) = matches.subcommand_matches("gen") {
        ::std::process::exit(match core::gen::gen() {
            Ok(_) => 0,
            Err(err) => {
                eprintln!("{}: Could not generate!!!\n\t{}: {:?}"
                    , Paint::red("ERROR").bold(), Paint::red("ERROR DESCRIPTION").bold(), err);
                1
            }
        });
    } else if let Some(_matches) = matches.subcommand_matches("blog") {
        /*
            "post" => err_handle("Not supported at the moment", false),
            "edit" => err_handle("Not supported at the moment", false),
            "list" => err_handle("Not supported at the moment", false),
            "delete" => err_handle("Not supported at the moment", false),
        */
    }
}
