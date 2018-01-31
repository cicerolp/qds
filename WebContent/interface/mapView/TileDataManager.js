class TileDataManager{
    constructor(placesInfoProvider, cacheSize){
	this.placesInfoProvider = placesInfoProvider;
	//
	this.tiles = {};
	this.insertionQueue = new Queue();
	this.tileCacheSize = cacheSize; 
	this.limitNumPlacesPerTile = 100;
	//
	this.DEUBG = true;
    }

    freeCache(fractionOfSize){
	if(this.insertionQueue.getLength() > this.tileCacheSize){
	    var numItemsToDelete = Math.floor(this.tileCacheSize * fractionOfSize);
	    for(var i = 0 ; i < numItemsToDelete ; ++i){
		var key = this.insertionQueue.dequeue();
		this.removeTile(key);
	    }
	}
    }
    
    addTile(tile,callback){
	var key = tile.x + "_" + tile.y + "_" + tile.z;
	//
	if(!(key in this.tiles)){
	    var middleTile = {"x":(tile.x+0.5),"y":(tile.y+0.5)};
	    var coordsTile = {"x":(tile.x),"y":(tile.y)}
	    //
	    var ctileLatLng = tile_to_degree(middleTile,tile.z);
	    var tileLatLng  = tile_to_degree(coordsTile,tile.z);
	    //
	    var latlng  = L.latLng(tileLatLng.lat, tileLatLng.lng);
	    var clatlng = L.latLng(ctileLatLng.lat, ctileLatLng.lng);
	    var radius = Math.floor(clatlng.distanceTo(latlng));
	    var limit = this.limitNumPlacesPerTile;
	    //
	    placesInfoProvider.getNearbyPlaces(ctileLatLng.lat,ctileLatLng.lng,radius,limit,(function(d){
		var places = [];
		if(d && d.response != ""){
		    var result = JSON.parse(d.response)	    	
		    if("places" in result) places = result["places"];
		}
		//console.log(result);
		this.tiles[key] = places;
		this.insertionQueue.enqueue(key);
		this.freeCache(.5);
		//
		if(callback) callback(null);
		
	    }).bind(this));
	}
	//console.log("add tile",key);
    }

    removeTile(key){
	delete this.tiles[key]
    }

    getPlaceByID(id,callback){
	placesInfoProvider.getInfo(id,function(placeInfo){
	    if(callback)
		callback(placeInfo);
	});
    }
    
    genRandomData(tile,callback){
	var key = tile.x + "_" + tile.y + "_" + tile.z;
	//
	if(!(key in this.tiles)){
	    var bottomLeftTileCoords = {"x":(tile.x),"y":(tile.y)};
	    var topRightTileCoords   = {"x":(tile.x+1),"y":(tile.y+1)};
	    //
	    var blTileLatLng  = tile_to_degree(bottomLeftTileCoords,tile.z);
	    var trTileLatLng  = tile_to_degree(topRightTileCoords,tile.z);
	    //
	    var numPoints = this.limitNumPlacesPerTile;
	    var places = d3.range(numPoints).map(function(d){
		var lambdaX = Math.random();
		var lambdaY = Math.random();

		var lng = blTileLatLng.lng + (trTileLatLng.lng - blTileLatLng.lng) * lambdaX;
		var lat = blTileLatLng.lat + (trTileLatLng.lat - blTileLatLng.lat) * lambdaY;

		return {"geo_location":{"lat":lat,"lng":lng},"name":"","labels":["restaurant"]};
	    });
	    //
	    //console.log(result);
	    this.tiles[key] = places;
	    this.insertionQueue.enqueue(key);
	    this.freeCache(.5);
	    //
	    if(callback) callback(null);

	}
	console.log("add tile",key);
    }
    
    getTileData(tile,callback){
	var key = tile.x + "_" + tile.y + "_" + tile.z;
	if(key in this.tiles)
	    callback(this.tiles[key]);
	else{
	    if(false){
		d3.queue()
    		    .defer((this.genRandomData).bind(this),tile)
		    .awaitAll((d=>{callback(this.tiles[key])}).bind(this));
	    }
	    else{
		d3.queue()
    		    .defer((this.addTile).bind(this),tile)
		    .awaitAll((d=>{callback(this.tiles[key])}).bind(this));
	    }
	}
    }

    getNumTiles(){
	return this.insertionQueue.getLength();
    }
}
