#include <unistd.h>

#include "config.hpp"
#include "filter.hpp"
#include "filters.hpp"
#include "video.hpp"

Config::Config( int argc, char *argv[], char *envp[ ] )
{
    parse_args( argc, argv, envp );
}

int Config::parse_args( int argc, char *argv[], char *envp[] )
{
    int opt;
    while ((opt = getopt(argc, argv, "f:i:v:")) != -1) {
        switch (opt) {
        case 'f':
            _filter_name = optarg;
            break;

        case 'i':
            _file_name = optarg;
            break;

        case 'v':
            _video_name = optarg;
            break;

        default:
            cerr << "ERROR unknown option: " << opt << endl;
            cerr << "Usage: vid [-v <videodev>] [-f <filter-name>] [-i <image-name>]" << endl;
            break;
        }
    }

    return 1;
}

Video*  Config::get_video()
{
    if ( _video_name == "" ) return NULL;
    return new Video( _video_name );
}

Filter* Config::get_filter()
{
    if ( _filter ) {
        return _filter;
    }

    if ( _filter_name == "" ) {
        return NULL;
    }

    _filters = new FltFilters();
    if ( _filters == NULL ) {
        return NULL;
    }
    
    _filter = _filters->get( _filter_name );
    return _filter;
}
