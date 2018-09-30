# FFIOCache
A cache module for FFmpeg, which USES custom avio as a proxy to cut into low-level data reads for caching

用于FFmpeg的一个缓存模块, 通过自定义avio作为代理,切入低层数据读取进行缓存

# Usage
  * 1.Download this project or files.
	1.下载本项目或文件

  * 2.Copy files to your project.
  	复制文件到你的项目中

  * 3.Modify your code of ffmpeg and rebuild it
	修改ffmpeg源代码并重新编译

    open libavformat/hls.c <br>
    find open_url_keepalive in the function open_url() <br>
    Add a condition in the front row:<br>
   	if(!(s->flags & AVFMT_FLAG_CUSTOM_IO && ..... )  //if custom is used,don`t keepalive <br>
  
	打开文件libavformat/hls.c <br>
	在open_url函数中找到open_url_keepalive的调用 <br>
	为上一行的if判断添加一个条件 <br>
	if(!(s->flags & AVFMT_FLAG_CUSTOM_IO && ..... )  //如果使用了自定义io, 不要调用keepalive <br>

  * 4.Replace function.
  	函数替换
    
    ---
    from:
    ```C
    AVFormatContext * fc = NULL;
    avformat_open_input(&fc, url, fmt,  opts);
    ```
    to:
    ```C
    AVFormatContext * fc = NULL;
    ff_io_cache_format_open_input(&fc, url, fmt,  opts, cache_flags);
    ```
    ---
    from:
    ```C
    avformat_free_context(fc);
    ```
    to:
    ```C
    ff_io_cache_format_free_context(fc);
    ```
    ---
    from:
    ```C
    avformat_close_input(fc);
    ```
    to:
    ```C
    ff_io_cache_format_close_input(fc);
    ```
    ---

  * 5.Other

  	Must call ff_io_cache_global_set_path before any
# Prompt

	Don`t set AVFormatContext->opaque
	不要修改AVFormatContext的opaque成员
	
	Don`t set AVFormatContext->io_open
	不要修改AVFormatContext的io_open函数指针
	
	Don`t set AVFormatContext->io_close
	不要修改AVFormatContext的io_close函数指针

	Don`t remove AVFMT_FLAG_CUSTOM_IO flag of AVFormatContext->flags
	不要去掉AVFormatCOntext的flags的AVFMT_FLAG_CUSTOM_IO标记

# Contact
	helei0908@hotmail.com
