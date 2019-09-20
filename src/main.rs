use std::env;
use yansi::Paint;

// Message macros
macro_rules! help_msg {
    () => {
        print!("mssg - Mart's Static Site Generator\n\
                \n\
                new  Create a new site project\n\
                init Create a new site project on the current directory\n\
                gen  Generate the static site\n\
                help Display this help message\n");
    };
}

macro_rules! err_handle {
    ($msg: expr) => {
        eprintln!("{}: {}", Paint::red("ERROR").bold(), $msg);
        help_msg!();
    };
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() == 1 {
        err_handle!("Need at least one argument!");
    } else {
        match &*args[1] {
            "new" => { err_handle!("Not supported yet"); }
            "init" => { err_handle!("Not supported yet"); }
            "gen" => { err_handle!("Not supported yet"); }
            "help" => help_msg!(),
            _ => { err_handle!("Non-valid argument!"); }
        };
    }
}
