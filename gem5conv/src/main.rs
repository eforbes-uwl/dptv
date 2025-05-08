  /* This file is part of the Dual PipeTrace Viewer (dptv) pipeline 
   * trace visualization tool. The dptv project was written by Adam 
   * Grunwald and Elliott Forbes, University of Wisconsin-La Crosse, 
   * copyright 2021-2025.
   *
   * dptv is free software: you can redistribute it and/or modify it
   * under the terms of the GNU General Public License as published 
   * by the Free Software Foundation, either version 3 of the License, 
   * or (at your option) any later version.
   *
   * dptv is distributed in the hope that it will be useful, but 
   * WITHOUT ANY WARRANTY; without even the implied warranty of 
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
   * General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License 
   * along with dptv. If not, see <https://www.gnu.org/licenses/>. 
   *
   *
   *
   * The dptv project can be found at https://cs.uwlax.edu/~eforbes/dptv/
   *
   * If you use dptv in your published research, please consider 
   * citing the following:
   *
   * Grunwald, A., Nguyen, P. and Forbes, E., "dptv: A New PipeTrace
   * Viewer for Microarchitectural Analysis," Proceedings of the 55th 
   * Midwest Instruction and Computing Symposium, April 2023. 
   *
   * If you found dptv helpful, please let us know! Email eforbes@uwlax.edu
   *
   * There are bound to be bugs, let us know those too.
   */

use std::io::{self, Read, Write};
use std::fs::{self, File};
use clap::Parser;
use flate2::bufread::GzDecoder;
use flate2::write::GzEncoder;
use flate2::Compression;
use param_trans::ParamTransEntry;
use trace_trans::gem5_to_dptv;

mod dptv_trace;
mod trace_trans;
mod param_trans;
mod cycle_bounds;


/// Simple program to greet a person
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Input gem5 trace file [default: input from stdin]
    #[arg(short, long, default_value_t = {"".to_string()}, hide_default_value = true)]
    input: String,

    /// Output dptv trace file [default: output to stdout]
    #[arg(short, long, default_value_t = {"".to_string()}, hide_default_value = true)]
    output: String,
    
    // Parameter list file [default: bulitin list, see source]
    #[arg(short, long, default_value_t = {"".to_string()}, hide_default_value = true)]
    params: String,

    /// gem5 cycle scale
    #[arg(short, long, default_value_t = 500)]
    scale: usize,

    /// Enable output gzip compression [default: false]
    #[arg(short, long, action, default_value_t = {"".to_string()}, hide_default_value = true)]
    comp: String,

    /// Enable input gzip decompression [default: false]
    #[arg(short, long, action, default_value_t = {"".to_string()}, hide_default_value = true)]
    decomp: String,
}

fn main() {
    
    // Parse cmd arguments
    let args = Args::parse();

    // Read gem5 trace from either stdin or file
    let input = if args.input.len() == 0 {
        // With no explicit arguments we read from stdin and write to stdout
        // this may not be the desired outcome for a first time user, so we print a message
        // informing them how to access the arguments list
        // Output to stderr so we don't conflict with the stdout trace output
        io::stderr().write(b"Reading gem5 trace from stdin\n").unwrap();
        io::stderr().write(b"Run with \"-i <INPUT>\" to read from a file\n").unwrap();
        io::stderr().write(b"or run with \"-h\" to print arguments list\n").unwrap();
        io::stderr().flush().unwrap();
        let mut stdin = io::stdin();
        let mut buf = Vec::new();
        stdin.read_to_end(&mut buf).unwrap();
        buf
    } else {
        if args.output.len() != 0 {
            println!("Reading gem5 trace");
        }
        let mut file = File::open(args.input).unwrap();
        let mut buf = Vec::new();
        file.read_to_end(&mut buf).unwrap();
        buf
    };
    
    // Decompress input
    let gem5_trace = if args.decomp.len() > 0 && args.comp.chars().next().unwrap().to_ascii_lowercase() == 't' {
        let mut d = GzDecoder::new(input.as_slice());
        let mut s = String::new();
        d.read_to_string(&mut s).unwrap();
        s
    } else {
        String::from_utf8(input).unwrap()
    };
    
    // Parse translation parameters, either from a file or the included list
    let mut params = if args.params.len() == 0 {
        ParamTransEntry::new(include_str!("params.txt"))
    } else {
        ParamTransEntry::new(&fs::read_to_string(args.params).unwrap())
    };
    
    // This is where the magic happens!
    if args.output.len() != 0 {
        println!("Converting gem5->dptv trace");
    }
    let trace = gem5_to_dptv(gem5_trace, args.scale, &mut params);
    
    // Encode trace as yaml
    let yaml = serde_yaml::to_string(&trace).unwrap();
    
    // Compress output
    let out = if args.comp.len() > 0 && args.comp.chars().next().unwrap().to_ascii_lowercase() == 't' {
        let mut e = GzEncoder::new(Vec::new(), Compression::default());
        e.write_all(yaml.as_bytes()).unwrap();
        e.finish().unwrap()
    } else {
        yaml.into_bytes()
    };
    
    // Output dptv trace to either stdout or file
    if args.output.len() == 0 {
        let mut stdout = io::stdout();
        stdout.write_all(&out).unwrap();
        stdout.flush().unwrap();
    } else {
        println!("Writing dptv trace");
        fs::write(args.output, out).unwrap();
    }
    
}
