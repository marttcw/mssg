use std::env;

mod core;

fn main() {
    let args: Vec<String> = env::args().collect();

    match args.len() {
        1 => core::msg::err_handle("Need at least one argument!", true),
        _ => {
            match &*args[1] {
                "help" => core::msg::help(),
                "ver"  => core::msg::version(),
                "init" => {
                    match args.len() {
                        2 => core::new::init("").unwrap(),
                        _ => core::new::init(&*args[2]).unwrap(),
                    }
                }
                "gen" => core::gen::gen().unwrap(),
                _ => core::msg::err_handle("Non-valid argument!", true),
            }
        }
    }
}
