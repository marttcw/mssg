use yansi::Paint;

mod core;

#[macro_use]
extern crate clap;

macro_rules! notsupported_message {
    () => {
        eprintln!("{}: Not supported at the moment.", Paint::red("ERROR").bold());
    }
}

macro_rules! exit_message {
    ($func:expr, $msg_couldnot:expr, $msg_description:expr) => {
         ::std::process::exit(match $func {
            Ok(_) => 0,
            Err(err) => {
                eprintln!("{}: Could not {}: {}\n\t{}: {:?}"
                    , Paint::red("ERROR").bold(), $msg_couldnot, $msg_description, Paint::red("ERROR DESCRIPTION").bold(), err);
                1
            }
        });       
    }
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

    match matches.subcommand_name() {
        Some("init") => {
            match matches.is_present("PROJ_NAME") {
                true => {
                    let proj_name = matches.value_of("PROJ_NAME").unwrap();
                    exit_message!(core::new::init(proj_name), "initialize", format!("Project \"{}\" already exists.", proj_name));
                }
                false => exit_message!(core::new::init(""), "initialize", "Project already exists."),
            }
        } 
        Some("gen") => exit_message!(core::gen::gen(), "generate", "See follow error description."),
        Some("blog") => {
            notsupported_message!();
            // post, edit, list, delete
        }
        Some("host") => notsupported_message!(),
        None => notsupported_message!(),
        _ => notsupported_message!(),
    }
}
