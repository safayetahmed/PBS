
    ffmpeg_sintel_dir = '../data/ffmpeg/dec.sintelfull.720p.mkv';

    freqconstmed_ns_dir = [ffmpeg_sintel_dir, '/freqconstmed/ns'];
    aseries_freqconstmed_ns = get_alpha_series(freqconstmed_ns_dir);

    freqcycle_ns_dir = [ffmpeg_sintel_dir, '/freqcycle/ns'];
    aseries_freqcycle_ns = get_alpha_series(freqcycle_ns_dir);

    freqconstmed_VIC_dir = [ffmpeg_sintel_dir, '/freqconstmed/VIC'];
    aseries_freqconstmed_VIC = get_alpha_series(freqconstmed_VIC_dir);

    freqcycle_VIC_dir = [ffmpeg_sintel_dir, '/freqcycle/VIC'];
    aseries_freqcycle_VIC = get_alpha_series(freqcycle_VIC_dir);
    
    figure;
    plot_alpha_series(aseries_freqconstmed_VIC, 'k');
    hold on;
    plot_alpha_series(aseries_freqcycle_VIC, 'b');
    plot_alpha_series(aseries_freqconstmed_ns, 'g');
    plot_alpha_series(aseries_freqcycle_ns, 'r');
    hold off;
    
    alpha = '1.2';
    
    %   frequency square waves VIC budget
    %   VFT_error      vs release_time
    %   cpuusage_ns    vs release_time
    %   cpuusage_VIC   vs release_time
    filename = [freqconstmed_ns_dir, '/', alpha, '/1_SRT_PeSoRTA_ffmpeg_dec.sintelfull.720p.mkv_6480_mavslmsbank_41666668_11000000_', alpha, '.log'];
    data_freqconstmed_ns = parse_csv_file(filename);
    VFT_eror_plot = plot_VFT_error(data_freqconstmed_ns, 'freqconstmed ns-based budget');
    
    %   frequency square waves VIC budget
    %   VFT_error      vs release_time
    %   cpuusage_ns    vs release_time
    %   cpuusage_VIC   vs release_time
    %   freqcycle_ns_dir
    filename = [freqcycle_ns_dir, '/', alpha, '/1_SRT_PeSoRTA_ffmpeg_dec.sintelfull.720p.mkv_6480_mavslmsbank_41666668_11000000_', alpha, '.log'];
    data_freqcycle_ns = parse_csv_file(filename);
    VFT_eror_plot = plot_VFT_error(data_freqcycle_ns, 'freqcycle ns-based budget');
    
    %   constant frequency ns budget
    %   VFT_error      vs release_time
    %   cpuusage_ns    vs release_time
    %   cpuusage_VIC   vs release_time
    filename = [freqconstmed_VIC_dir, '/', alpha, '/1_SRT_PeSoRTA_ffmpeg_dec.sintelfull.720p.mkv_6480_mavslmsbank_41666668_11000000_', alpha, '.log'];
    data_freqconstmed_VIC = parse_csv_file(filename);
    VFT_eror_plot = plot_VFT_error(data_freqconstmed_VIC, 'freqconstmed VIC-based budget');
        
    %   frequency square waves VIC budget
    %   VFT_error      vs release_time
    %   cpuusage_ns    vs release_time
    %   cpuusage_VIC   vs release_time
    filename = [freqcycle_VIC_dir, '/', alpha, '/1_SRT_PeSoRTA_ffmpeg_dec.sintelfull.720p.mkv_6480_mavslmsbank_41666668_11000000_', alpha, '.log'];
    data_freqcycle_VIC = parse_csv_file(filename);
    VFT_eror_plot = plot_VFT_error(data_freqcycle_VIC, 'freqcycle VIC-based budget');
    