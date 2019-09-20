use std::env;

mod core;

fn main() {
    let args: Vec<String> = env::args().collect();

    match args.len() {
        1 => core::msg::err_handle("Need at least one argument!", true),
        _ => {
            match &*args[1] {
                "help" => core::msg::help(),
                "init" => {
                    match args.len() {
                        2 => core::new::init(""),
                        _ => core::new::init(&*args[2]),
                    }
                }
                "gen" => core::msg::err_handle("Not supported yet", true),
                _ => core::msg::err_handle("Non-valid argument!", true),
            }
        }
    }
}
