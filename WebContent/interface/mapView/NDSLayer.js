function toNanocubeCoords(coords){
    var latlng             = tile_to_degree(coords,coords.z,false);
    var nanocubeTileCoords = degree_to_tile(latlng,coords.z,true);
    //return z + "," + nanocubeTileCoords.x + "," + nanocubeTileCoords.y;
    return {"x":nanocubeTileCoords.x, "y": nanocubeTileCoords.y, "z": coords.z};
}

L.GridLayer.CanvasCircles = L.GridLayer.extend({
    createTile: function (coords,done) {

        var tile = document.createElement('canvas');

        var tileSize = this.getTileSize();
        tile.setAttribute('width', tileSize.x);
        tile.setAttribute('height', tileSize.y);
        var ctx = tile.getContext('2d');
	var resolution = 10;

	//
	var query = new NDSQuery(datasetName,"count",spatialDimension,function(result,myQ){
	    //
	    var resolution = 5;
	    var numPixels = 2**resolution;
	    var pixelWidth  = tileSize.x / numPixels;
	    var pixelHeight = tileSize.y / numPixels;		
	    
	    result.forEach(function(pixel){
		
		ctx.fillRect(pixel[0]*pixelWidth, (numPixels-pixel[1] - 1)*pixelHeight , pixelWidth, pixelHeight);
	    });
	    
            done(null, tile);	// Syntax is 'done(error, tile)'
	});
	query.addConstraint("tile",spatialDimension,{"x":coords.x,"y":coords.y,"z":coords.z,"resolution":resolution});
	ndsInterface.query(query);
	
        return tile;
    	
    }
});

L.gridLayer.canvasCircles = function(opts) {
    return new L.GridLayer.CanvasCircles(opts);
};


L.GridLayer.CanvasLayer = L.GridLayer.extend({
    createTile: function (coords, done) {
	if(true){
	    var tile = document.createElement('canvas');

            var tileSize = this.getTileSize();
            tile.setAttribute('width', tileSize.x);
            tile.setAttribute('height', tileSize.y);

            var ctx = tile.getContext('2d');

            // Draw whatever is needed in the canvas context
            // For example, circles which get bigger as we zoom in
            ctx.beginPath();
            ctx.arc(tileSize.x/2, tileSize.x/2, 4 + coords.z*4, 0, 2*Math.PI, false);
            ctx.fill();
            return tile;
	}
    }
});

//http://10.0.100.237:8585/q(visits.b('location',dive(tile2d(4,3,7),2),'img8'))
//http://10.0.100.237:8585/q(visits.b('location',dive(tile2d(10,412,488),0),'img8'))

L.gridLayer.canvasLayer = function(opts) {
    return new L.GridLayer.CanvasLayer(opts);
};

L.GridLayer.DebugCoords = L.GridLayer.extend({
    createTile: function (coords) {
        var tile = document.createElement('div');
        tile.innerHTML = [coords.x, coords.y, coords.z].join(', ');
        tile.style.outline = '1px solid red';
        return tile;
    }
});

L.gridLayer.debugCoords = function(opts) {
    return new L.GridLayer.DebugCoords(opts);
};


class NDSLayer{

    constructor(containerMap, ndsInterface, minZoomLevel, maxZoomLevel){
	//
	this.containerMap = containerMap;
	this.ndsInterface = ndsInterface;

	//
	this.minNormalizer = 0.0;
	this.maxNormalizer = 1.0;

	//
	this.colorScale = d3.scaleQuantile().domain([0,1]).range(['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#b10026']);
	
	//
	var that = this;
	this.tileLayer = L.gridLayer.canvasCircles({"ndsInterface":ndsInterface, "parent":that});
	 this.tileLayer.addTo(this.containerMap);

	//
	this.debugLayer = L.gridLayer.debugCoords();
	this.debugLayer.addTo(this.containerMap);
    }
    
    setOpacity(newValue){
	this.tileLayer.setOpacity(newValue);
    }

    setColorNormalization(newMin,newMax){
	this.minNormalizer = newMin;
	this.maxNormalizer = newMax;
	this.tileLayer.setExtent([newMin,newMax]);
	this.tileLayer.updateTiles();
    }

    setEnabled(v){
	this.enabled = v;
	if(this.enabled){
	    this.tileLayer.addTo(this.containerMap);
	}
	else{
	    this.tileLayer.removeFrom(this.containerMap);
	}
    }
    
    renderData(){
	//console.log("Nanocube layer: render");
    }
}
