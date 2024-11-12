dptv: The Dual Pipe-Trace Viewer
================================

The Dual Pipe-Trace Viewer (dptv) is a processor pipeline event visualization tool. A processor simulator will record pipeline events, and generate a trace file. The dptv viewer tool is written in C, using the SDL2 graphical package. dptv will display the pipeline events in a pipeline timing diagram, such that the user can easily zoom and pan to navigate the dynamic instruction steam. Similar tools have been developed and published in prior literature. The key feature, not found in any other tools, is the ability to display two traces such that the same benchmark has been simulated on different processor configurations, even if the two configurations operate at different clock frequencies. dptv therefore makes it much easier to compare performance trade-offs when exploring architectural alternatives.

If you use dptv in your own research, please consider citing our MICS paper cited below. And if you find dptv helpful, please let me know (eforbes@uwlax.edu) - I'd like to have a rough idea of how many people have tried it.

Grunwald, A., Nguyen, P. and Forbes, E., "dptv: A New PipeTrace Viewer for Microarchitectural Analysis," Proceedings of the 55th Midwest Instruction and Computing Symposium, March 2023.
