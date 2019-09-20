use std::env;

mod core;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 1 {
        core::msg::err_handle("Need at least one argument!");
    } else {
        match &*args[1] {
            "help" => core::msg::help(),
            "new" => core::msg::err_handle("Not supported yet"),
            "init" => core::msg::err_handle("Not supported yet"),
            "gen" => core::msg::err_handle("Not supported yet"),
            _ => core::msg::err_handle("Non-valid argument!"),
        };
    }
}
