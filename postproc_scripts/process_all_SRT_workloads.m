
rootdir = '~/Desktop/log2/ffmpeg/';

matname_list = ...
                {   'dec.arthur.amr.mat', ... 
                    'dec.arthur.mp3.mat', ... 
                    'dec.arthur.opus.mat', ... 
                    'dec.cc.amr.mat', ... 
                    'dec.cc.opus.mat', ... 
                    'dec.deadline.flv.mat', ... 
                    ... %'dec.deadline.mp4.mat', ... 
                    'dec.deadline.webm.mat', ... 
                    'dec.dsailors.mp3.mat', ... 
                    'dec.factory.flv.mat', ... 
                    'dec.factory.mp4.mat', ... 
                    ... %'dec.factory.webm.mat', ... 
                    'dec.highway.flv.mat', ... 
                    'dec.highway.mp4.mat', ... 
                    'dec.highway.webm.mat', ... 
                    'dec.mozart.mp3.mat', ... 
                    'dec.sintel.mp3.mat', ... 
                    'enc.arthur.wav.amr.mat', ... 
                    'enc.arthur.wav.mp3.mat', ... 
                    'enc.arthur.wav.opus.mat', ... 
                    'enc.cc.wav.amr.mat', ... 
                    'enc.cc.wav.opus.mat', ... 
                    'enc.deadline.y4m.flv.mat', ... 
                    'enc.deadline.y4m.mp4.mat', ... 
                    'enc.deadline.y4m.webm.mat', ... 
                    'enc.dsailors.wav.mp3.mat', ... 
                    'enc.highway.y4m.flv.mat', ... 
                    'enc.highway.y4m.mp4.mat', ... 
                    'enc.highway.y4m.webm.mat', ... 
                    'enc.mad900.y4m.flv.mat', ... 
                    'enc.mad900.y4m.mp4.mat', ... 
                    'enc.mad900.y4m.webm.mat', ... 
                    'enc.mozart.wav.mp3.mat', ... 
                    'enc.sintel.wav.mp3.mat'};

dir_list_size = length(matname_list);

for i = 1:dir_list_size
    dirpath = strcat(rootdir, matname_list{i});
    [h1, h2, h3] = process_SRT_runset(dirpath);
    close(h1);
    close(h2);
    close(h3);
end
