module.exports = function(grunt) {

  // Project configuration.
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    // Install all the packages listed in the bower.json file.
    shell: {
      bower_install: {
         command: 'bower install'
      }
    },
    // Copy all the bower files into a single directory.
    bower: {
      dev: {
        dest: '../../../out/grunt/third_party'
      }
    },
    // Concatenate all the files in third_party into a single file.
    concat: {
      dist: {
        src: [
          '../../../out/grunt/third_party/WeakMap.js',
          '../../../out/grunt/third_party/classlist.js',
          '../../../out/grunt/third_party/pointerevents-polyfill.js',
          '../../../out/grunt/third_party/MutationObserver.js',
          '../../../out/grunt/third_party/CustomElements.js',
          '../../../out/grunt/third_party/HTMLImports.js',
        ],
        dest: '../../../out/grunt/src/<%= pkg.name %>.js'
      }
    },
    // Uglify the one big file into one smaller file.
    uglify: {
      options: {
        banner: '/*! <%= pkg.name %> built from /exerimental/webtry/poly <%= grunt.template.today("yyyy-mm-dd") %> */\n'
      },
      build: {
        src: '../../../out/grunt/src/<%= pkg.name %>.js',
        dest: '../res/js/<%= pkg.name %>.js'
      }
    },
    copy: {
      simple: {
        src: '../../../out/grunt/src/<%= pkg.name %>.js',
        dest: '../res/js/<%= pkg.name %>.js'
      }
    }
  });

  // Load the plugins for the above commands.
  grunt.loadNpmTasks('grunt-bower');
  grunt.loadNpmTasks('grunt-contrib-concat');
  grunt.loadNpmTasks('grunt-contrib-copy');
  grunt.loadNpmTasks('grunt-contrib-cssmin');
  grunt.loadNpmTasks('grunt-contrib-uglify');
  grunt.loadNpmTasks('grunt-shell');

  // By default run all the commands in the right sequence to build our custom minified polyfill.
  grunt.registerTask('default', ['shell:bower_install', 'bower', 'concat', 'uglify']);

  // A target to build an unminified version, for debugging.
  grunt.registerTask('notmin', ['shell:bower_install', 'bower', 'concat', 'copy:simple']);

};
