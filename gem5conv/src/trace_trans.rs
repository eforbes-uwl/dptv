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


use crate::{cycle_bounds::CycleInstrBounds, dptv_trace::{Inst, Param, Stage, Trace}, param_trans::ParamTransEntry};


const SYS_DECODE: &'static str = "system.cpu.decoder";






pub fn gem5_to_dptv(gem5: String, scale: usize, param_trans: &mut [ParamTransEntry]) -> Trace {
    
    // Read trace twice: first to get the list of instructions and stages (lines starting with O3PipeView),
    // second to insert any identified parameters (any other line)
    
    
    
    // First Pass
    let mut trace = Trace {
        insts: Vec::new()
    };
    // Current instruction
    let mut cur_inst = Inst {
        tid: 0,
        pc: String::new(),
        micro_pc: 0,
        text: String::new(),
        stages: Vec::new(),
    };
    let mut cycle_bounds = CycleInstrBounds::new();
    let mut cur_seq_num = 0;
    let mut min_cycle = 0;
    let mut max_cycle = 0;
    // Iterate over lines
    for line in gem5.split('\n') {
        // Check if this line is part of the O3PipeView trace
        if line.starts_with("O3PipeView:") {
            let (_, stages) = line.split_once(":").unwrap();    // Always succeedes: previous test verified existance of colon
            // Fetch stage is handled differently
            if stages.starts_with("fetch") {
                // Previous instruction must be finished, add it to the trace
                trace.add_inst(cur_inst, cur_seq_num);
                cur_inst = Inst::default();
                cycle_bounds.add(min_cycle, max_cycle, cur_seq_num);
                // Fetch format: fetch:<cycle>:<pc>:<micro pc>:<sequence number>:<instruction type>:<instruction text>
                //println!("{}", stages);
                let mut fetch_iter = stages.split(':');
                let fetch = fetch_iter.next().unwrap();
                let cycle = fetch_iter.next().unwrap().parse::<usize>().unwrap() / scale;
                let pc = fetch_iter.next().unwrap();
                let micro_pc = fetch_iter.next().unwrap().parse::<usize>().unwrap();
                let seq_num = fetch_iter.next().unwrap().parse::<usize>().unwrap() - 1;
                let inst_type = fetch_iter.next().unwrap();
                let inst_text = fetch_iter.next().unwrap_or("nop");
                // Set parameters in instruction
                cur_inst.pc = pc.to_string();
                cur_inst.micro_pc = micro_pc;
                cur_inst.text = inst_type.to_string() + " : " + inst_text;
                cur_inst.stages.push(Stage {
                    cycle: cycle,
                    name: fetch.to_string(),
                    id: fetch.as_bytes()[0] as char,    // TODO
                    color: 7,   // Legacy
                    params: Vec::new(),
                });
                cur_seq_num = seq_num;
                min_cycle = cycle;
                max_cycle = cycle;
            } else {
                // Other stage formats: <stage>:<cycle>
                let (stage, cycle) = stages.split_once(':').unwrap();
                // Remove trailing : if it exists
                let cycle = match cycle.split_once(':') {
                    Some((cycle, _)) => cycle,
                    None => cycle,
                };
                let cycle = cycle.parse::<usize>().unwrap() / scale;
                if cycle == 0 {
                    // A cycle of 0 indicates that this stage was skipped
                    continue;
                }
                // Get stage character id
                // Currently this is just the first character of the name, with hardcoded exceptions for stage names which share
                // the first letter with another name
                let mut stage_id = stage.as_bytes()[0] as char;
                if stage == "dispatch" { stage_id = 'D'; }
                if stage == "retire" { stage_id = 'R'; }
                // Add stage
                cur_inst.stages.push(Stage {
                    cycle: cycle,
                    name: stage.to_string(),
                    id: stage_id,
                    color: 7,   // Legacy
                    params: Vec::new(),
                });
                min_cycle = min_cycle.min(cycle);
                max_cycle = max_cycle.max(cycle);
            }
        }
    }
    trace.add_inst(cur_inst, cur_seq_num);
    cycle_bounds.add(min_cycle, max_cycle, cur_seq_num);
    
    
    // Second Pass
    let mut split = gem5.split('\n');
    let mut cur_decode = 0;
    // Iterate over lines
    loop {
        if let Some(line) = split.next() {
            // Simply continue to next line if parsing this one fails
            let (cycle, rest) = match line.split_once(':') {
                Some(v) => v,
                None => continue,
            };
            let cycle = cycle.trim_start();
            let cycle = match cycle.parse::<usize>() {
                Ok(v) => v,
                Err(_) => continue,
            } / scale;
            let (flag, line) = match rest.split_once(':') {
                Some(v) => v,
                None => continue,
            };
            let flag = flag.trim_start();
            // Special processing for Decode parameters
            let mut processed_decoder = false;
            if flag.contains(SYS_DECODE) {
                // Extract decode parameters
                let mut params: Vec<Param> = Vec::new();
                // First line: Decode: Decoded <inst> instruction: 
                if let Some((_left, right)) = line.split_once("Decode: Decoded ") {
                    if let Some((left, _right)) = right.split_once(" instruction") {
                        params.push(Param {
                            name: "Instr: ".to_string(),
                            value: left.to_string(),
                        });
                        processed_decoder = true;
                        // Next lines: <name> = <value>
                        // Return to regular parsing when a line ends with a closed bracket
                        loop {
                            let line = split.next().unwrap();
                            if let Some((name, value)) = line.split_once('=') {
                                // Remove leading/trailing spaces, brackets, and commas from <value>
                                let value = value.replace(['{', '}', ',', '\t'], "");
                                let value = value.trim_start().trim_end();
                                let name = name.trim_start().trim_end();
                                if value.len() > 0 {
                                    params.push(Param {
                                        name: name.to_string(),
                                        value: value.to_string(),
                                    });
                                }
                            }
                            if line.ends_with("}") {
                                break;
                            }
                        }
                        // Add params to appropriate instruction / instructions
                        // Instructions are always decoded in-order
                        // On some architectures (such as X86) one decoded instruction will get split into multiple executed instructions
                        // Therefore we want to add these parameters to every instruction that origonated from this single decoded instruction
                        // We will add these parameters to every instruction until the instruction has a Micro PC parameter of 0 (excluding the first one)
                        let mut first = true;
                        loop {
                            if let Some(inst) = trace.insts.get_mut(cur_decode) {
                                if !first && inst.micro_pc == 0 {
                                    break;
                                }
                                // Add parameters to decode
                                for stage in &mut inst.stages {
                                    if stage.name == "decode" {
                                        stage.params.extend_from_slice(&params);
                                    }
                                }
                                first = false;
                                cur_decode += 1;
                            } else {
                                break;
                            }
                        }
                    }
                }
            }
            if !processed_decoder {
                // General processing for non-decode parameters
                // Currenly we only use the timestamp and the flag to determine where to place parameters
                // The flag determines what type of stage to insert into, ex: system.cpu.rename gets placed into the rename stages
                // Currently params will be applied to every stage of that type that occured on that cycle.
                
                for entry in param_trans.iter_mut() {
                    
                    if !flag.starts_with(&entry.flag) {
                        continue;
                    }
                
                    // Check if any regexes match this line
                    // Each capturing group in each regex provides the name of the parameter and allows for capturing the parameter value
                    for regex in entry.regexes.iter() {
                        // Iterate over captures
                        for cap in regex.captures_iter(line) {
                            // Get the sequence number (default: 0 - write this parameter to every stage)
                            if let Some(d) = cap.name("sn_all") {
                                entry.cur_sn = d.as_str().parse::<usize>().unwrap();
                            }
                            let sn = if entry.cur_sn == 0 {
                                match cap.name("sn") {
                                    Some(d) => d.as_str().parse::<usize>().unwrap(),
                                    None => 0,
                                }
                            } else {
                                entry.cur_sn
                            };
                            // Iterate over capturing groups
                            for name in regex.capture_names() {
                                if let Some(name_str) = name {
                                    // If the name contains a + or a -, that is an offset for what the actual value should be
                                    let mut name = name_str.to_string();
                                    let mut value_offset = 0;
                                    if let Some((lname, rname)) = name_str.split_once("PLUS") {
                                        name = lname.to_string();
                                        value_offset = rname.parse().unwrap_or(0);
                                    } else if let Some((lname, rname)) = name_str.split_once("MINUS") {
                                        name = lname.to_string();
                                        value_offset = -rname.parse().unwrap_or(0);
                                    }
                                    if let Some(m) = cap.name(name_str) {
                                        // Found a match!
                                        let str_param_value = m.as_str();
                                        let param_value: String;
                                        // If param is a number add the value offset to it
                                        if let Ok(num) = str_param_value.parse::<isize>() {
                                            param_value = format!("{}", (num + value_offset));
                                        } else {
                                            param_value = str_param_value.to_string();
                                        }
                                        if sn == 0 {
                                            // Insert into stages at the current cycle
                                            // Searching through section of instruction list which might have stages at this cycle
                                            for i in cycle_bounds.inst_range(cycle) {
                                                let inst = &mut trace.insts[i];
                                                for stage in &mut inst.stages {
                                                    if stage.cycle == cycle && stage.name == entry.stage_name {
                                                        // Found a stage this parameter can go in
                                                        stage.params.push(Param {
                                                            name: name.clone(),
                                                            value: param_value.clone(),
                                                        });
                                                    }
                                                }
                                            }
                                            // Done iterating over instructions to check
                                        } else if name != "sn" && name != "sn_all" {
                                            // Insert into instruction defined by sn
                                            if let Some(inst) = trace.insts.get_mut(sn-1) {
                                                let mut found_stage = false;
                                                let any_name = entry.stage_name == "any";
                                                for stage in &mut inst.stages {
                                                    // If going to specific stage, go into that stage regardless of cycle
                                                    // If can go into any stage, then go into the one with the correct cycle
                                                    if (!any_name && stage.name == entry.stage_name) || (any_name && stage.cycle == cycle) {
                                                        // Found a stage this parameter can go in
                                                        stage.params.push(Param {
                                                            name: name.clone(),
                                                            value: param_value.clone(),
                                                        });
                                                        found_stage = true;
                                                    }
                                                }
                                                // If no matching stage was found and parameter could have gone into any stage,
                                                // add a dummy stage for the parameter
                                                if !found_stage && any_name {
                                                    // Find place to insert new stage, insert before stage of higher cycle number
                                                    let mut insert = 0;
                                                    loop {
                                                        if inst.stages.len() <= insert { break; }
                                                        if inst.stages[insert].cycle > cycle { break; }
                                                        insert += 1;
                                                    }
                                                    inst.stages.insert(insert, Stage {
                                                        cycle,
                                                        name: "DUMMY INFO".to_string(),
                                                        id: 'X',
                                                        color: 7,
                                                        params: vec![Param {
                                                            name: name.clone(),
                                                            value: param_value.clone(),
                                                        }]
                                                    });
                                                }
                                            }
                                            
                                        }
                                    }
                                }
                            }
                            // Done iterating over capture groups
                        }
                    }
                    // Done iterating over regexes
                }
            }
        } else {
            // Done iterating over lines
            break;
        }
    }
    
    
    // Return dptv trace
    trace
}




