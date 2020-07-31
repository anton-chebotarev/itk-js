from flask import Flask

# set the project root directory as the static folder, you can set others.
app = Flask(__name__, static_url_path='')

@app.route('/Test.js')
def send_js():
    return app.send_static_file('Test.js')
    
@app.route('/')
def root():
    return app.send_static_file('Test.html')

if __name__ == "__main__":
    app.run()