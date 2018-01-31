//load data
var dataInFile = {};

function latLongToPixelXY(latitude, longitude) {
    var pi_180 = Math.PI / 180.0;
    var pi_4 = Math.PI * 4;
    var sinLatitude = Math.sin(latitude * pi_180);
    var pixelY = (0.5 - Math.log((1 + sinLatitude) / (1 - sinLatitude)) / (pi_4)) * 256;
    var pixelX = ((longitude + 180) / 360) * 256;

    var pixel = { x: pixelX, y: pixelY };

    return pixel;
}

function loadPointJSON(filename,key,callback){
    dataInFile[key] = {"vertexCoords":[],"colorCoords":[],"dataInfo":{"numPoints":0}}
    
    d3.json(filename,function(dataOBJ){
	var data         = dataOBJ["data"];
	var numObjects   = data.length;
	var offset           = 0;
	var arrayIndexOffset = 0;

	dataInFile[key].dataInfo.numPoints = data.length;
	numClasses = placeClasses.length;

	ctr = 0
	
	for(var _index in data){
	    var elt = data[_index];
	    var colorIndex = placeToIndex[elt["class"]];
	    var colorCoef = colorIndex*((1.0/numClasses)*1.5);
	    //var elt = latLongToPixelXY(eltt.x,eltt.y);
	    //not storing names for now
	    dataInFile[key].vertexCoords.push(+(elt.y));
	    dataInFile[key].vertexCoords.push(+(elt.x));
	    dataInFile[key].colorCoords.push(colorCoef);
	    ctr += 1
	}
	console.log("Number of Points ",ctr);
	callback(null);
    })
}

function loadPolygonJSON(filename,key,callback){
    dataInFile[key] = {"vertexCoords":[],"colorCoords":[],"dataInfo":{"numPoints":0}}
    
    d3.json(filename,function(dataOBJ){
	var data         = dataOBJ["data"];
	var numObjects   = data.length;
	var offset           = 0;
	var arrayIndexOffset = 0;

	dataInFile[key].dataInfo.numPoints = data.length;
	numClasses = placeClasses.length;

	ctr = 0
	
	for(var _index in data){
	    var elt = data[_index];
	    var colorIndex = placeToIndex[elt["class"]];
	    var colorCoef = colorIndex*((1.0/numClasses)*1.5);
	    //var elt = latLongToPixelXY(eltt.x,eltt.y);
	    //not storing names for now
	    dataInFile[key].vertexCoords.push(+(elt.y));
	    dataInFile[key].vertexCoords.push(+(elt.x));
	    dataInFile[key].colorCoords.push(colorCoef);
	    ctr += 1
	}
	console.log("Number of Points ",ctr);
	callback(null);
    })
}

d3.queue()
    .defer(loadPointJSON,"../places.2.json","file3")
    .defer(loadPointJSON,"../location.1.545.json","file3")
    .awaitAll(initializeSystem);

function initializeSystem(){
    //crete map
    var mapID = "mapID";
    d3.select("body").append("div").attr("id",mapID);
    
var tileURL = 'http://{s}.tiles.wmflabs.org/bw-mapnik/{z}/{x}/{y}.png';//"http://{s}.sm.mapstack.stamen.com/(toner-background,$fff[difference],$fff[@23],$fff[hsl-saturation@20],toner-lines[destination-in])/{z}/{x}/{y}.png";
    var tileLayerProperties = {
	maxZoom: 18,
	attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
    };//{maxZoom: 18,attribution: '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'};
    var myMap = new GLLeafletMap(mapID,[-8.038893,-34.891078], 10, tileURL, tileLayerProperties);
    myMap.addPointLayer("pointLayer0",dataInFile["file3"].dataInfo,dataInFile["file3"].vertexCoords,dataInFile["file3"].colorCoords);
    myMap.updateLegend(placeClassColorScale,"categorical","Place Class");
}
