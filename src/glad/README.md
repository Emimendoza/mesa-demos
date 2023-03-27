This directory was generated as follows:

    git clone -b glad2 https://github.com/Dav1dde/glad
    cd glad
    python -m glad --api=egl=,gl:core=,gl:compatibility=,gles1=,gles2=,wgl= --merge --out-path /path/to/mesa/demos/src/glad c --loader
