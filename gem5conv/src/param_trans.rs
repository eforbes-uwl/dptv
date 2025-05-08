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

use regex::Regex;


pub struct ParamTransEntry {
    pub flag: String,
    pub stage_name: String,
    pub regexes: Vec<Regex>,
    pub cur_sn: usize,
}
impl ParamTransEntry {
    pub fn new(text: &str) -> Vec<ParamTransEntry> {
        
        let mut list: Vec<ParamTransEntry> = Vec::new();
        
        let mut cur_entry = ParamTransEntry {
            flag: String::new(),
            stage_name: String::new(),
            regexes: Vec::new(),
            cur_sn: 0,
        };
        
        for line in text.split('\n') {
            // Ignore commented lines
            if line.starts_with("#") { continue; }
            
            if line.starts_with("-") {
                // Start of a new flag group
                let (_, line) = line.split_once('-').unwrap();
                let line = line.trim_start();
                let (lhs, rhs) = line.split_once(':').unwrap();
                let lhs = lhs.trim_end();
                let rhs = rhs.trim_start();
                // Push old entry
                if cur_entry.flag.len() > 0 {
                    list.push(cur_entry);
                }
                // Setup new entry
                cur_entry = ParamTransEntry {
                    flag: lhs.to_string(),
                    stage_name: rhs.to_string(),
                    regexes: Vec::new(),
                    cur_sn: 0,
                };
            } else if line.len() > 0 {
                // Regex Entry
                let regex = Regex::new(line).unwrap();
                cur_entry.regexes.push(regex);
            }
        }
        
        // Push last entry
        if cur_entry.flag.len() > 0 {
            list.push(cur_entry);
        }
        
        list
    }
}

