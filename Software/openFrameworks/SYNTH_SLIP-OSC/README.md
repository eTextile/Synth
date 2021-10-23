# eTextile-Synthesizer 
### Exploring music through textile

## Requirements

### openFrameworks 0.11.2

    https://github.com/openframeworks/openFrameworks/releases/tag/0.11.2

To compile the code, you need to set the following variable in your ~/.bashrc, ~/.zshrc or other equivalent:

    # UPDATE WITH YOUR OWN OPEN FRAMEWORKS PATH:
    export OF_ROOT=/opt/openFrameworks/

    cd ${OF_ROOT}
    git submodule update --init
    git submodule update

The E256-openframeworks program requires the following addons:
 - ofxGui
 - ofxOscSerial

The ofxOscSerial addon is not include by default in openFrameworks

    cd ${OF_ROOT}/addons
    git clone https://github.com/bakercp/ofxOscSerial.git

## Compiling E256-openframeworks
    make

If you have 4 cores, it's recommended to use them all as it's long:

    make -j4

To accelerate the compilation we use ccache, you can get it with:

    apt-get install ccache  # debian-like OS
    brew install ccache     # OSX

### Running (at least Linux & Mac)
    make RunRelease

## TODO

