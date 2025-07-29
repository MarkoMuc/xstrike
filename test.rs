use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::process;

fn main() {
    let file_path = "/dev/xstrike";
    let mut file = match OpenOptions::new()
        .read(true)
        .write(true)
        .create(false)
        .open(file_path)
    {
        Ok(f) => f,
        Err(e) => {
            eprintln!("test-dev: Failed to open {} with {}", file_path, e);
            process::exit(1);
        }
    };

    let write_data = b"Hello world from the rust to kernel driver\n";
    match file.write_all(write_data) {
        Ok(_) => {}
        Err(e) => eprintln!("test-dev WE: Error writing to device : {}", e),
    }

    let mut read_data = [0; 1024];
    match file.read(&mut read_data) {
        Ok(bytes_read) => match String::from_utf8(read_data[..bytes_read].to_vec()) {
            Ok(s) => print!("test-dev R: {}", s),
            Err(e) => eprintln!("test-dev RE: {} {:?}", e, read_data),
        },
        Err(e) => eprintln!("Error reading from device : {}", e),
    }
}
