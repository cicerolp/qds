    getVisibleTilesCoords(){
	var map = this.containerMap;
	// get bounds, zoom and tileSize        
	var bounds = map.getPixelBounds();
	var zoom = map.getZoom();
	var tileSize = 256;
	var tileCoordsContainer = [];
	// get NorthWest and SouthEast points
	var nwTilePoint = new L.Point(Math.floor(bounds.min.x / tileSize),
				      Math.floor(bounds.min.y / tileSize));
	var seTilePoint = new L.Point(Math.floor(bounds.max.x / tileSize),
				      Math.floor(bounds.max.y / tileSize));
	// get max number of tiles in this zoom level
	var max = map.options.crs.scale(zoom) / tileSize; 
	// enumerate visible tiles 
	for (var x = nwTilePoint.x; x <= seTilePoint.x; x++) 
	{
            for (var y = nwTilePoint.y; y <= seTilePoint.y; y++) 
            {
		var xTile = Math.abs(x % max);
		var yTile = Math.abs(y % max);
		
		tileCoordsContainer.push({ 'x':xTile, 'y':yTile , 'z':zoom});
		//console.log('tile ' + xTile + ' ' + yTile);
            }
	}
	
	return tileCoordsContainer;
    }

    codeTileDataForRendering(key){
	//
	var numClasses = placeClasses.length;
	//
	var keys = Object.keys(pointsTest);
	var x = pointsTest;
	var vertexCoords = [];
	var colorCoords = [];
	var numPoints = 0;
	placesLoaded = [];
	keys.forEach(key=>{
	    var places = pointsTest[key];
	    places.forEach(place=>{
		placesLoaded.push(place);
		var label = ("labels" in place)?place["labels"][0]:"NULL";
		var colorIndex = placeToIndex[getClass(label)];
		var colorCoef = colorIndex*(1.0/numClasses) + 0.5/numClasses;		
		vertexCoords.push(place.geo_location.lat);
		vertexCoords.push(place.geo_location.lng);
		colorCoords.push(colorCoef);
		numPoints += 1;
	    });
	});
	console.log("NUM POINTS", numPoints);
	myMap.getLayer("Places Layer").setData({"numPoints":numPoints},vertexCoords,colorCoords); 
	myMap.repaint();
    }

    codeAllTileDataForRendering(keys){
    }
