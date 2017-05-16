import os.path
import gzip
import tarfile

TARGZ_DEFAULT_COMPRESSION_LEVEL = 9

def make_tarball(tarball_path, sources, base_dir, prefix_dir=''):
    """Parameters:
    tarball_path: output path of the .tar.gz file
    sources: list of sources to include in the tarball, relative to the current directory
    base_dir: if a source file is in a sub-directory of base_dir, then base_dir is stripped
        from path in the tarball.
    prefix_dir: all files stored in the tarball be sub-directory of prefix_dir. Set to ''
        to make them child of root.
    """
    base_dir = os.path.normpath( os.path.abspath( base_dir ) )
    def archive_name( path ):
        """Makes path relative to base_dir."""
        path = os.path.normpath( os.path.abspath( path ) )
        common_path = os.path.commonprefix( (base_dir, path) )
        archive_name = path[len(common_path):]
        if os.path.isabs( archive_name ):
            archive_name = archive_name[1:]
        return os.path.join( prefix_dir, archive_name )
    def visit(tar, dirname, names):
        for name in names:
            path = os.path.join(dirname, name)
            if os.path.isfile(path):
                path_in_tar = archive_name(path)
                tar.add(path, path_in_tar )
    compression = TARGZ_DEFAULT_COMPRESSION_LEVEL
    tar = tarfile.TarFile.gzopen( tarball_path, 'w', compresslevel=compression )
    try:
        for source in sources:
            source_path = source
            if os.path.isdir( source ):
                os.path.walk(source_path, visit, tar)
            else:
                path_in_tar = archive_name(source_path)
                tar.add(source_path, path_in_tar )      # filename, arcname
    finally:
        tar.close()

def decompress( tarball_path, base_dir ):
    """Decompress the gzipped tarball into directory base_dir.
    """
    # !!! This class method is not documented in the online doc
    # nor is bz2open!
    tar = tarfile.TarFile.gzopen(tarball_path, mode='r')
    try:
        tar.extractall( base_dir )
    finally:
        tar.close()
