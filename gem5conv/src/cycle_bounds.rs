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

use std::ops::Range;



// Tool for getting the range of instruction ids which could have a stage at this cycle 
pub struct CycleInstrBounds {
    // id: cycle
    vec: Vec<(usize, usize)>,
}
impl CycleInstrBounds {
    pub fn new() -> Self {
        Self {
            vec: Vec::new(),
        }
    }
    pub fn add(&mut self, min_cycle: usize, max_cycle: usize, seq_num: usize) {
        // Fill default values up to max_cycle
        while self.vec.len() <= max_cycle {
            self.vec.push((usize::MAX, usize::MIN));
        }
        for i in min_cycle..=max_cycle {
            let entry = &mut self.vec[i];
            entry.0 = entry.0.min(seq_num);
            entry.1 = entry.1.max(seq_num);
        }
    }
    pub fn inst_range(&self, cycle: usize) -> Range<usize> {
        if cycle >= self.vec.len() {
            0 .. 0
        } else {
            self.vec[cycle].0 .. (self.vec[cycle].1 + 1)
        }
    }
}

