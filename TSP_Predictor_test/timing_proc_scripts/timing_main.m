
    timing_summary_dir = '../timing_data_summary/';
    mkdir(timing_summary_dir);

    timing_dir = '../timing_data/Core2_extreme/'
    rootdir_filelist = readdir(timing_dir);
    rootdir_filecount= length(rootdir_filelist);
    
    %loop through each file in the root directory
    for d1 = 1:rootdir_filecount
            
        %get the file name
        subdir_name = rootdir_filelist{d1};
        subdir_path = sprintf('%s/%s', timing_dir, subdir_name);
        
        %check if it is a directory
        if ~ isdir(subdir_path)
            %not a directory. move onto the next file
            'not a dir'
            continue;
        end
        
        %make sure the directory is not . or ..
        if strcmp(subdir_name, '.') || strcmp(subdir_name, '..')
            %if it is either the . or .. directory, move onto the next file
            subdir_name
            continue;
        end
        
        %each valid directory is the name of an application
        application = subdir_name
        
        %make a directory to store the output
        application_summary_dir = sprintf('%s/%s', timing_summary_dir, application);
        mkdir(application_summary_dir);
        
        config_filelist = readdir(subdir_path);
        config_filecount= length(config_filelist);
        %loop through each file in the subdirectory
        for d2 = 1:config_filecount
        
            %get the file name
            subsubdir_name = config_filelist{d2};
            subsubdir_path = sprintf('%s/%s', subdir_path, subsubdir_name);

            %check if it is a directory
            if ~ isdir(subsubdir_path)
                %not a directory. move onto the next file
                continue;
            end

            %make sure the directory is not . or ..
            if strcmp(subsubdir_name, '.') || strcmp(subsubdir_name, '..')
                %if it is either the . or .. directory, move onto the next file
                continue
            end

            %each valid directory is the name of a configuration
            config = subsubdir_name
            
            %get summarized data on the directory
            summary = process_timing_wrkloads(  timing_dir, ...
                                                application, ...
                                                config);
            
            %set the output file name
            outfile_name = sprintf('%s/%s.%s.mat',  application_summary_dir, ...
                                                    application, ...
                                                    config);
            
            %save the summary structure to the output file name
            save(outfile_name, 'summary');
            
        end
    end

