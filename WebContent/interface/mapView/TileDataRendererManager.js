var placeClasses = ["Sustenance","Education","Transportation","Financial","Healthcare","Entertainment","Religion","Others"];
var placeToIndex = {"Sustenance":0,"Education":1,"Transportation":2,"Financial":3,"Healthcare":4,"Entertainment":5,"Religion":6,"Others":7}
var placeClassColorScale = d3.scaleOrdinal().domain(placeClasses).range(["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#fdbf6f", "#ff7f00"]);

class TileDataRendererManager{
    constructor(containerMap, tileDataManager, numTiles, zoomLevel, placeSelectionCallback){
	this.containerMap = containerMap;
	this.labelSetFilter = new Set([]);
	this.selectedMarker = undefined;
	this.zoomLevel = zoomLevel;
	//
	var that = this;
	this.defaultStyle = {
	    "weight" : 0.5,
	    "fillOpacity" : 0.7,
	    "radius" : 10
	};
	//
	this.markers = d3.range(numTiles * tileDataManager.limitNumPlacesPerTile).map(function(d){
	    var marker = L.circleMarker([0,0], {
		color: 'black',
		weight: that.defaultStyle.weight,
		fillColor: '#bebada',
		fillOpacity: that.defaultStyle.fillOpacity,
		radius: that.defaultStyle.radius
	    });
	    
	    marker.on("click",function(e){
		if(placeSelectionCallback){
		    placeSelectionCallback(e.target.place,that.zoomLevel);
		}
		if(that.selectedMarker != undefined){
		    that.selectedMarker.setStyle({"color":"black","weight":that.defaultStyle.weight});
		}
		that.selectedMarker = e.target;
		that.selectedMarker.setStyle({"color":"white","weight":6*that.defaultStyle.weight});
	    });
	    
	    return marker;
	});

	//
	containerMap.on("zoomend",function(e){
	    var zoom = containerMap.getZoom();
	    console.log(zoom);
	    if(zoom >= that.zoomLevel){
	    }
	    else
		that.markers.forEach(marker=>marker.removeFrom(containerMap));
	});
	
	//
	this.colorMode = "None";
	this.buffer = d3.range(numTiles).map(function(d){return {"tile":{"x":-1,"y":-1,"z":-1},"active":false}});
	this.waitingQueue = new Queue();
	this.tileDataManager = tileDataManager;
	//
	this.dataInfo = d3.range(numTiles).map(function(d,i){ return {"numPoints":0,"startIndex":(i*tileDataManager.limitNumPlacesPerTile)} });

	//
	//setInterval(f, 5*60*1000);
    }

    getAvailablePosition(){
	var bufferSize = this.buffer.length;
	for(var i = 0; i < bufferSize ; ++i){
	    if(!(this.buffer[i].active))
		return i;
	}
	return undefined;    
    }

    addTileToRender(tile){
	//make sure it only renders the intended zoom level
	if(this.zoomLevel != tile.z)
	    return;
	//
	var that = this;
	var position = this.getAvailablePosition();
	//console.log("*******************available position", position,this.isTileVisible(tile));
	
	if(position != undefined && this.isTileVisible(tile)){
	    var key = tile.x + "_" + tile.y + "_" + tile.z;
	    this.buffer[position].active = true;
	    this.buffer[position].tile = key;
	    
	    if(false){//webgl
		this.tileDataManager.getTileData(tile,function(tileData){		
		    //console.log("tileData",tileData);
		    // //get data tile and put it in the buffer
		    // //already coded??
    		    var numClasses = placeClasses.length;
		    //
    		    var vertexCoords = [];
    		    var colorCoords = [];
    		    var numPoints = 0;
		    //
    		    tileData.forEach(place=>{
    	    		var label = ("labels" in place)?place["labels"][0]:"NULL";
    	    		var colorIndex = placeToIndex[getClass(label)];
    	    		var colorCoef = colorIndex*(1.0/numClasses) + 0.5/numClasses;		
    	    		vertexCoords.push(place.geo_location.lat);
    	    		vertexCoords.push(place.geo_location.lng);
    	    		colorCoords.push(colorCoef);
    	    		numPoints += 1;
    		    });
		    
		    //myMap.getLayer("Places Layer").setSubData(,,); 
    		    // myMap.getLayer("Places Layer").setData({"numPoints":numPoints},vertexCoords,colorCoords); 
    		    // myMap.repaint();
		});
	    }
	    else{
		var that = this;
		var numPlacesPerTile = this.tileDataManager.limitNumPlacesPerTile;
		this.tileDataManager.getTileData(tile,function(tileData){		
		    //console.log("tileData",tileData);
		    // //get data tile and put it in the buffer
		    // //already coded??
    		    var numClasses = placeClasses.length;
		    //
    		    tileData.forEach((place,i)=>{
			var markerIndex = (position*numPlacesPerTile)+i;

			//set lat lng
			var lat = (place.geo_location.lat);
			var lng = (place.geo_location.lng);
			var newLatLng = new L.LatLng(lat, lng);
			that.markers[markerIndex].setLatLng(newLatLng);

			//set style
			var labels = place.labels;
			if(labels == undefined){
			    labels = ["NULL"];
			    place.labels = ["NULL"];
			}
			var label = labels[0];
			var labelColor = "black";
			if(label != "NULL"){
			    var labelClass = getClass(label);
			    labelColor = placeClassColorScale(labelClass);
			}
			that.markers[markerIndex].labelColor = labelColor;
			var style = {
			    color: "black",			    
			    fillColor: labelColor
			};
			
			if(that.colorMode == "None"){
			    style.color = 'black';
			    style.fillColor = '#bebada';
			}
			that.markers[markerIndex].setStyle(style);
			
			//set place
			that.markers[markerIndex].place = place;
			//
			if(that.markerPassesLabelFilter(that.markers[markerIndex]) && (that.containerMap.getZoom() >= tile.z))
			    that.markers[markerIndex].addTo(that.containerMap);
			else
			    that.markers[markerIndex].removeFrom(that.containerMap);
		    });
		});
		//that.containerMap.redraw();
	    }
	}
	else{
	    this.waitingQueue.enqueue(tile);
	}
    }
    
    setMarkerColorMode(colorMode){
	this.colorMode = colorMode;
	if(colorMode == "Label"){	
	    this.markers.forEach(myMarker=>{
		var style = {
		    fillColor: myMarker.labelColor
		};
		
		myMarker.setStyle(style);
	    });	    
	}
	else if(colorMode == "None"){
	    var style = {
		fillColor: '#bebada'
	    };

	    this.markers.forEach(myMarker=>{
		myMarker.setStyle(style);
	    });	    
	}
    }

    markerPassesLabelFilter(myMarker){
	var that = this;
	//
	if(this.labelSetFilter.size == 0){
	    //add makers for active tiles
	    return true;
	}
	else{
	    //remove markers
	    if(!("place" in myMarker)){
		return false;
	    }
	    
	    //
	    var place = myMarker.place;
	    if(!("labels" in place))
		return false;
	    //
	    else{
		var labels = place.labels;
		if(labels.length == 0)
		    debugger
		var hasLabel = false;
		labels.forEach(label=>{
		    if(that.labelSetFilter.has(label))
			hasLabel = true;
		});
		//
		return hasLabel;
	    }
	}
    }
    
    filterLabel(labelSet){
	this.labelSetFilter = labelSet;
	var that = this;
	this.markers.forEach((myMarker,i)=>{
	    if(that.markerPassesLabelFilter(myMarker)){
		//myMarker.(that.containerMap)
		var tileIndex = Math.floor((1.0*i) / (tileDataManager.limitNumPlacesPerTile) );
		var tileInfo = this.buffer[tileIndex];
		if(tileInfo.active)
		    myMarker.addTo(that.containerMap)
	    }
	    else{
		myMarker.removeFrom(that.containerMap)
	    }
	});
    }
    
    removeTileFromRenderList(tile){
	//console.log("##############3remove tile");
	var key = tile.x + "_" + tile.y + "_" + tile.z;
	var bufferSize = this.buffer.length;
	var position = -1;
	for(var i = 0; i < bufferSize ; ++i){
	    if(this.buffer[i].tile == key){
		this.buffer[i].active = false;
		position = i;
		break;
	    }
	}
	//
	if(position >= 0){
	    // for(var i = 0; i < bufferSize ; ++i){
	    // 	if(this.buffer[i].tile == key){
	    // 	    this.buffer[i].active = false;
	    // 	    position = i;
	    // 	    break;
	    // 	}
	    // }
	}
	//
	this.processNextInWaitingList();
    }

    processNextInWaitingList(){
	if(this.waitingQueue.getLength() == 0)
	    return;
	
	var tile = this.waitingQueue.dequeue();

	if(this.isTileVisible(tile)){
	    //if visible render
	    this.addTileToRender(tile);
	}
	//else: discard
    }

    isTileVisible(tile){
	var visibleTiles = this.getVisibleTilesCoords();
	var numVisibleTiles = visibleTiles.length;
	for(var i = 0 ; i < numVisibleTiles ; ++i){
	    var vTile = visibleTiles[i];
	    if(vTile == undefined || tile == undefined)
		debugger
	    if(tile.x == vTile.x && tile.y == vTile.y && tile.z == vTile.z)
		return true;
	}
	return false;
    }
    
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

    queryPlaceByID(placeID){
	this.tileDataManager.getPlaceByID(placeID,(function(place){
	    //render selected place
	    console.log(place);
	    //debugger
	    //set selection
	    //center map
	    this.containerMap.setView(place.geo_location,this.zoomLevel);
	}).bind(this));
    }

    clearSelection(){
	this.markers.forEach((function(marker){
	    marker.setStyle({"color":"black","weight":this.defaultStyle.weight});
	}).bind(this));
	this.selectedMarker = undefined;
    }    
}
