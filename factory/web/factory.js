Module['onRuntimeInitialized'] = function() {
  const canvas = document.getElementById('canvas');
  const context = canvas.getContext('2d');

  const setup = Module.cwrap('setup', null, []);
  const loop = Module.cwrap('loop', null, []);
  const getScreen = Module.cwrap('getScreen', 'number', []);
  const setInputs = Module.cwrap('setInputs', null, ['number']);
  const consumeDelay = Module.cwrap('consumeDelay', 'number', [])

  const keys = ['arrowleft','arrowright','arrowup','arrowdown','x','z'];
  let inputs = 0x3f;
  let bufferedInputs = 0x3f;

  window.addEventListener('keydown', e => {
    let buttonIndex = keys.indexOf(e.key.toLowerCase());
    if (buttonIndex === -1) return;

    let mask = ~(1 << buttonIndex);
    inputs &= mask;
    bufferedInputs &= mask;
  });

  window.addEventListener('keyup', e => {
    let buttonIndex = keys.indexOf(e.key.toLowerCase());
    if (buttonIndex === -1) return;

    inputs |= 1 << buttonIndex;
  });

  setInputs(inputs);
  setup();

  let screenPtr = getScreen();

  let screen = new Uint8Array(Module.HEAPU8.buffer, screenPtr, (48*84) >> 3);

  function render() {
    context.fillStyle = '#c3d6c2';
    context.fillRect(0, 0, canvas.width, canvas.height);

    context.fillStyle = '#141c11';
    
    let scale = 10;

    for (let y = 0; y < 48; y++) {
      for (let x = 0; x < 84; x++) {
        let byteIndex = 84*(y >> 3) + x;
        let bitIndex = y & 7;
        if (screen[byteIndex] & (1 << bitIndex)) {
          context.fillRect(scale*x, scale*y, scale-1, scale-1);
        }
      }
    }
  }

  render();

  function update() {
    let t0 = Date.now();
    setInputs(bufferedInputs);
    bufferedInputs |= inputs;
    loop();
    render();
    
    let t1 = Date.now();
    let delay = consumeDelay() - (t1 - t0);
    delay = Math.max(delay, 1);
    setTimeout(update, delay);
  }

  setTimeout(update, consumeDelay());
}


// Filesystem functions.
const fileSystem = {};

function fileExists(pathPtr) {
  let path = UTF8ToString(pathPtr);
  let exists = fileSystem[path] ? 1 : 0;
  return exists;
}

function fileRead(pathPtr, pos, dest, size) {
  let path = UTF8ToString(pathPtr);
  let file = fileSystem[path];
  if (!file) return 0;

  if (pos >= file.length) return 0;

  size = Math.min(size, file.length-pos);

  let destArray = new Uint8Array(Module.HEAPU8.buffer, dest, size);
  destArray.set(file.array.subarray(pos, pos + size));

  return size;
}

function fileWrite(pathPtr, pos, src, size) {
  let path = UTF8ToString(pathPtr);

  let file = fileSystem[path];
  if (!file) {
    file = {
      length: 0,
      array: new Uint8Array(0),
    };
    fileSystem[path] = file;
  }

  if (pos + size > file.array.length) {
    let newCapacity = Math.max(pos+size, 2*file.array.length);
    let array = new Uint8Array(newCapacity);
    array.set(file.array);
    file.array = array;
  }

  let srcArray = new Uint8Array(Module.HEAPU8.buffer, src, size);
  file.array.set(srcArray, pos);

  file.length = Math.max(file.length, pos + size);

  console.log('fileWrite', path, pos, src, size);
  console.log(file);

  return size;
}

function fileFlush(path) {
}

function fileRemove(path) {
  delete fileSystem[path];
}

function fileRename(srcPtr, destPtr) {
  let srcPath = UTF8ToString(srcPtr);
  let destPath = UTF8ToString(destPtr);

  let file = fileSystem[srcPath];
  fileSystem[destPath] = fileSystem[srcPath];
  delete fileSystem[srcPath];
  
  persistFilesystem();
}

function loadFilesystem() {
  let data = localStorage.getItem('fs');
  if (!data) return;

  let fileStrs = data.split(';');
  for (let fileStr of fileStrs) {
    let [path, content] = fileStr.split(':');
    let file = {
      array: Uint8Array.fromBase64(content),
    };
    file.length = file.array.length;
    fileSystem[path] = file;
  }
}

function persistFilesystem() {
  let fileStrs = [];
  for (let path of Object.keys(fileSystem)) {
    let file = fileSystem[path];
    let content = file.array.subarray(0, file.length).toBase64();
    fileStrs.push(`${path}:${content}`);
  }
  localStorage.setItem('fs', fileStrs.join(';'));
}

loadFilesystem();
