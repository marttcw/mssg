use std::{env};
use actix_files as fs;
use actix_web::{middleware, App, HttpServer};

pub fn run() -> std::io::Result<()> {
    println!("Hosting at: http://127.0.0.1:8080/");

    env_logger::init();

    HttpServer::new(|| {
        App::new()
            // enable logger
            .wrap(middleware::Logger::default())
            .service(
                // static files
                fs::Files::new("/", "./build/").index_file("index.html"),
            )
    })
    .bind("127.0.0.1:8080")?
    .run()
}
