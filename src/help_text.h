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

#ifndef HELP_TEXT_WHOOPEE
#define HELP_TEXT_WHOOPEE




// Help Text
int help_page = 0;
const int num_help_pages = 7;
const char*** help_text = (const char**[]){
(const char*[]){
"  ==== Dual Pipetrace Viewer ====",
" ",
"dptv is a processor timing trace",
"viewer. The left window shows a",
"list of instructions, in the order",
"they were fetched. The right window",
"show the pipeline stages that each",
"instruction visited. Each column in",
"the right window represents one",
"clock cycle. Characters in the",
"right window are used to indicate",
"the processor stages, place the",
"cursor over a stage for details",
"in the upper right parameter box",
" ",
"Press h to toggle help menu",
"Arrow keys and wasd to change page",
""},
(const char*[]){
"             Movement",
" ",
"Left-click to drag the camera view",
" ",
"Use arrow keys and wasd to move the",
"camera view by one unit at a time",
" ",
"Hold shift while moving the camera",
"view to move quickly, in proportion",
"to the current view scale",
" ",
"Hover over the stage area and",
"scroll to zoom in and out",
" ",
"Press q or Q to exit",
" ",
"(NOTE: All key controls listed",
"assume a QWERTY layout)",
""},
(const char*[]){
"          Two-Trace Mode",
" ",
"When working with two traces, each",
"trace will be interleaved together",
"in the display area",
" ",
"The two traces being compared are",
"expected to have their cycle",
"positions desync at some point",
"To resync them at a specific point",
"in executeion, right click on a",
"point in the stages windowt to snap",
"their cycle positions together at",
"that instruction",
" ",
"You can also use z/x to manually",
"shift the second trace",
""},
(const char*[]){
"          Basic Searching",
" ",
"Press / to begin searching",
"Type in your search string then",
"press ENTER to initiate search",
" ",
"Press n to continue searching down",
"Press N to continue searching up",
" ",
"Press ESC while typing in your",
"search or during the search to",
"cancel",
" ",
"The program will search for matches",
"in each instruction text, each",
"stage text, and in each stage's",
"parameters",
""},
(const char*[]){
"        Advanced Searching",
" ",
"You can also search only in a",
"specific field. After pressing /",
"type in the character for the field",
"you want to search in, then type",
"another / and enter your search",
" ",
"For example, you can type:",
"/i/<something>",
"to search only in the instruction",
"fields",
" ",
"In addition, to search for a",
"parameter value in a specific",
"parameter, type:",
"/v:<param name>/<param value>",
""},
(const char*[]) {
"     Advanced Searching (cont)",
" ",
"The following are the character",
"codes for each search section:",
" ",
"i: Instruction Text",
"p: PC Text",
"s: stage text",
"d: stage identifier",
"n: stage parameter name",
"v: stage parameter value",
""},
(const char*[]){
"        Additional Controls",
" ",
"Press r to reset view to its",
"state upon startup",
" ",
"Press p to snap your view to the",
"begining cycle of the top",
"instruction",
" ",
"Press P (upper case) to toggle",
"continual snapping",
""
}};


#endif
