from flask import Flask

# set the project root directory as the static folder, you can set others.
app = Flask(__name__, static_url_path='')

@app.route('/12bit_images.ihdr')
def send_ihdr():
    return app.send_static_file('12bit_images.ihdr')

@app.route('/12bit_images.hevc')
def send_hevc():
    return app.send_static_file('12bit_images.hevc')
    
@app.route('/12bit_images_lossless.hevc')
def send_hevc_lossless():
    return app.send_static_file('12bit_images_lossless.hevc')
    
@app.route('/Vector.js')
def send_vjs():
    return app.send_static_file('Vector.js')

@app.route('/ImageStack.js')
def send_js():
    return app.send_static_file('ImageStack.js')

@app.route('/ImageStackWasm.js')
def send_wasm_js():
    return app.send_static_file('ImageStackWasm.js')

@app.route('/ImageStackWasm.wasm')
def send_wasm():
    return app.send_static_file('ImageStackWasm.wasm')
 
@app.route('/')
def root():
    return app.send_static_file('ImageStack.html')

if __name__ == "__main__":
    app.run()