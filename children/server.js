const express = require('express');
const path = require('path');
const app = express();
const port = 5000;

// URL rewrite logic
app.use((req, res, next) => {
    // Split the path into segments
    const segments = req.path.split('/').filter(Boolean); 
    console.log(segments)

    if (segments.length > 1) {
        // More than one segment: replace the last '/' with an underscore and append '.html'
        const lastSegment = segments.pop();
        console.log(lastSegment);
        req.url = '/' + segments.join('/') + '_' + lastSegment + '.html';
        console.log(req.url);
    } else if (segments.length === 1) {
        // Single segment: just append '.html'
        req.url = '/' + segments[0] + '.html';
    } // If there are no segments, it will be handled by the root route ('/')

    next();
});

// Serve static files from 'children' directory
app.use(express.static('children'));

// Serve dmca-notice.html for the root URL ('/')
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'children', 'dmca-notice.html'));
});

// Serve dmca-notice.html for any 404 Not Found URLs
app.use((req, res, next) => {
    res.status(404).sendFile(path.join(__dirname, 'children', 'dmca-notice.html'));
});

app.listen(port, () => {
    console.log(`Server listening at http://localhost:${port}`);
});


