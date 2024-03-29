
    screen_size = get(0, 'ScreenSize');
    f1 = figure;
    set(f1, 'Position', [0 0 screen_size(3) screen_size(4) ] );
    plot1_name = '../plots/missrate_vs_budget.jpg';
    
    f2 = figure;
    set(f2, 'Position', [0 0 screen_size(3) screen_size(4) ] );
    plot2_name = '../plots/missrate_vs_RMSVFTerror.jpg';
    
    root_dir = '../data/ffmpeg/';
    workload_list = {   'dec.arthur.spx', ...
                        'dec.mozart.mp3', ...
                        'dec.sintel.mp4', ...
                        'enc.arthur.wav.amr', ...
                        'enc.beethoven.wav.mp3', ...
                        'enc.highway.y4m.mp4'};
    
    outfile_name = 'alphas.txt';
    fid = fopen(outfile_name, 'w');
    
    workload_count = 6;
    
    colors = {[1.0, 0, 0], [0, 0, 1.0], [0, 0, 0]};
        
    for w_i = 1:workload_count
    
        workload_dir = sprintf('%s/%s', root_dir, workload_list{w_i});
        series_set = get_series_set(workload_dir);
        
        fprintf(fid, '%s: \n', disp(workload_list{w_i}));
        for s_i = 1:length(series_set.series_names)
            fprintf(fid, '\t%s: \n', series_set.series_names{s_i});
            
            [alphas, sort_i]= sort(series_set.alphas{s_i});
            mean_miss_rates = series_set.mean_miss_rates{s_i}(sort_i);
            budget          = series_set.mean_allocated_budget{s_i}(sort_i);
            
            for a_i = 1:length(series_set.alphas{s_i})
                fprintf(fid, '\t%f,\t%f,\t%f\n',alphas(a_i), ...
                                                mean_miss_rates(a_i), ...
                                                budget(a_i));
            end
        end

        figure(f1);
        subplot(2, 3, w_i);
        [handles] = scatter_series_set( series_set.series_names, ...
                                        series_set.miss_rates, ...
                                        series_set.allocated_budget, ...
                                        colors);
        title(workload_list{w_i});
        
        figure(f2);
        subplot(2, 3, w_i);
        [handles] = scatter_series_set( series_set.series_names, ...
                                        series_set.miss_rates, ...
                                        series_set.RMS_VFT_error, ...
                                        colors);
        title(workload_list{w_i});
    end
    
    print(f1, plot1_name, '-djpg');
    print(f2, plot2_name, '-djpg');
    
    fclose(fid);

