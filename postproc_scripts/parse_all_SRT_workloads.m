
rootdir = '~/Desktop/log2/ffmpeg/';

dirname_list = ...
                {   'dec.arthur.amr', ... 
                    'dec.arthur.mp3', ... 
                    'dec.arthur.opus', ... 
                    'dec.cc.amr', ... 
                    'dec.cc.opus', ... 
                    'dec.deadline.flv', ... 
                    ... %'dec.deadline.mp4', ... 
                    'dec.deadline.webm', ... 
                    'dec.dsailors.mp3', ... 
                    'dec.factory.flv', ... 
                    'dec.factory.mp4', ... 
                    ... %'dec.factory.webm', ... 
                    'dec.highway.flv', ... 
                    'dec.highway.mp4', ... 
                    'dec.highway.webm', ... 
                    'dec.mozart.mp3', ... 
                    'dec.sintel.mp3', ... 
                    'enc.arthur.wav.amr', ... 
                    'enc.arthur.wav.mp3', ... 
                    'enc.arthur.wav.opus', ... 
                    'enc.cc.wav.amr', ... 
                    'enc.cc.wav.opus', ... 
                    'enc.deadline.y4m.flv', ... 
                    'enc.deadline.y4m.mp4', ... 
                    'enc.deadline.y4m.webm', ... 
                    'enc.dsailors.wav.mp3', ... 
                    'enc.highway.y4m.flv', ... 
                    'enc.highway.y4m.mp4', ... 
                    'enc.highway.y4m.webm', ... 
                    'enc.mad900.y4m.flv', ... 
                    'enc.mad900.y4m.mp4', ... 
                    'enc.mad900.y4m.webm', ... 
                    'enc.mozart.wav.mp3', ... 
                    'enc.sintel.wav.mp3'};

dir_list_size = length(dirname_list);

for i = 1:dir_list_size
    dirpath = strcat(rootdir, dirname_list{i});
    parse_SRT_runsdir(dirpath);
end

