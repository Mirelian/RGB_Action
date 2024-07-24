const http = require('http');
const fs = require('fs');
const path = require('path');

// Configuration
const PORT = 3000;
const firmwarePath = path.join(__dirname, '..', '.pio', 'build', 'esp32-s3-devkitc-1', 'firmware.bin');

const server = http.createServer((req, res) => {
  console.log("getting a request");
  if (req.method === 'GET' && req.url === '/firmware.bin') {
    fs.stat(firmwarePath, (err, stats) => {
      if (err) {
        console.error('File not found:', err);
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('File not found');
        return;
      }

      res.writeHead(200, {
        'Content-Type': 'application/octet-stream',
        'Content-Length': stats.size
      });

      const readStream = fs.createReadStream(firmwarePath);
      readStream.pipe(res);
    });
  } else {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not Found');
  }
});

server.listen(PORT, () => {
  console.log(`HTTP server is listening on port ${PORT}`);
});