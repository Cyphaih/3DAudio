This is just a small project I am making out of interest in 3D Audio and in learning some new programming skills.

I am using Windows COM for the audio 
-----
For now I have implemented the following:


Reading Input and rendering Output. 
Implemented a Three step Input - Process - Output system for the audio in different threads.
Output currently is a flat delay between left and right to setup the 3D System and check if its working.

-----
Optimizations:

The threads are currently working on Sleep timers instead of events from COM 
Audio can not catch up when behind for any reason
AudioEngine contains Buffers and Variables that I want to move elsewehere later.
-----
Planned next steps:

1. Save Input into Mono and process audio to sound like its in 3D space in stereo
2. Change Output to receive multiple different 3D Originators for sound 
3. Add another functionality to Use Input to create samples I can replay later and place in space
4. Do the Same with mp3 and wav files
5. Create an Audio scene 