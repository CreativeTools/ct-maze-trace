"Maze Trace" plugin for CINEMA 4D
=============

#[Download](https://github.com/CreativeTools/ct-maze-trace/archive/master.zip)
![Screenshot](https://raw.github.com/CreativeTools/ct-softbox-shader/master/screenshot.png)
###[Video](https://vimeo.com/70096750)

##Compability
The plugin has only been tested with C4D R14.

##Installation
Unzip the folder to your _CINEMA/plugins/_ folder and restart CINEMA.
After extraction the files (cinema4dsdk.cdl, cinema4dsdk.vcproj, etc.) should lie in _CINEMA/plugins/Maze Trace/_.

##Usage
The plugin contains two objects that can be found under the _plugins->Maze Trace_ menu entry, _Maze Trace_ and _Point Generator_.
###Maze Trace
A generator (place objects under it) that takes polygon objects and connects their points in interesting ways (it tries naïvely to solve the Traveling Salesman Problem by connecting each point to its nearest neighbor).

###Point Generator
A helper object that generates random points on the surface of an input object. Since the output object only consists of points it will not show up in the 3d view.

##Settings
###Maze Trace
* Maximum Segment Length
  * Delete segments longer than this. Useful for getting rid of occasional lines connecting different parts of the model.
* Relative
  * When checked the _Maximum Segment Length_ is a multiplier of neighboring points, otherwise it's an absolute length.

###Point Generator
* Num Points
  * The number of points to generate on the surface of the object
