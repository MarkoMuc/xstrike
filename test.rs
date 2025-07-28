use std::fs::OpenOptions;
use std::io::{ Read, Write};
use std::process;

fn main() {
    let file_path = "/dev/xstrike";
    let mut file = match OpenOptions::new()
        .read(true)
        .write(true)
        .create(false)
        .open(file_path) {
            Ok(f) => f,
            Err(e) => {
                eprintln!("test-dev: Failed to open {} with {}", file_path, e);
                process::exit(1);
            }
        };

    // println!("test-dev: Opened {}", file_path);

    let write_data = b"Hello world from the rust to kernel driver\n";
    match file.write_all(write_data) {
        // Ok(_) => println!("test-dev: Wrote to the driver"),
        Ok(_) => {},
        Err(e) => eprintln!("test-dev: Error writing to device : {}", e)
    }

    let mut read_data = [0; 1024];
    match file.read(&mut read_data) {
        Ok(bytes_read) => { 
            // println!("test-dev: Read {}B from the driver", bytes_read);
           match String::from_utf8(read_data[..bytes_read].to_vec()) {
                Ok(s) => println!("test-dev: {}",s),
                Err(_) => eprintln!("test-dev: {:?}",read_data)
            }
        },
        Err(e) => eprintln!("Error reading from device : {}", e)
    }
}
