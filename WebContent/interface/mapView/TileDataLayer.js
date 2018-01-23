function toInternalCoords(coords){
    var latlng             = tile_to_degree(coords,coords.z,false);
    var internalTileCoords = degree_to_tile(latlng,coords.z,true);
    return {"x":internalTileCoords.x, "y": internalTileCoords.y, "z": coords.z};
}

L.GridLayer.CanvasLayer = L.GridLayer.extend({
    extent: [Infinity,-Infinity],
    opactiy:0.5,
    updateExtent(valExtent){
	var layer = this;
	var newMin = d3.min([valExtent[0],layer.extent[0]]);
	var newMax = d3.max([valExtent[1],layer.extent[1]]);
	if(newMin == newMax){
	    newMax += 1;
	}
	layer.extent = [newMin,newMax];
    },
    setExtent(newExtent){
	this.extent = newExtent;
    },
    setOpacity(v){
	this.opacity = v;
	this.updateTiles();
    },
    updateTiles:function(){
	console.log("update tiles");
        var tileSize = this.getTileSize();
	var globalExtent = this.extent;
	var colorScale = this.options.parent.colorScale;
	var layer = this;
	
	for(var tileKey in this._tiles){
	    var tile = this._tiles[tileKey];
	    var canvas = tile.el;
	    var ctx = canvas.getContext('2d');
	    ctx.clearRect(0, 0, canvas.width, canvas.height);
	    
	    if(tile.el.data && tile.el.data.values.length > 0){
		//
		var resolution = 5;
		var numPixels = 2**resolution;
		var pixelWidth  = tileSize.x / numPixels;
		var pixelHeight = tileSize.y / numPixels;		
		var colors = placeClassColorScale.range();
		var result = tile.el.data;

		var opacityStr = (255*layer.options.opacity).toString(16);
		
		result.pixels.forEach((pixel,index)=>{			
		    var value = (result.values[index] - globalExtent[0]) / (globalExtent[1] - globalExtent[0]);
		    var rgba = colorScale(value) + opacityStr;
		    ctx.fillStyle = rgba;
		    ctx.fillRect(pixel[0]*pixelWidth, (numPixels-pixel[1] - 1)*pixelHeight , pixelWidth, pixelHeight);
		});
		
	    }
	};
    },
    myColorScale:function(count){
	var lc = Math.log(count + 1) / Math.log(10);

        var r = Math.floor(256 * Math.min(1, lc));
        var g = Math.floor(256 * Math.min(1, Math.max(0, lc - 1)));
        var b = Math.floor(256 * Math.min(1, Math.max(0, lc - 2)));

        return "rgba(" + r + "," + g + "," + b + "," + 1 + ")";
    },
    createTile: function (coords, done) {
        var tile = document.createElement('canvas');
	tile.data = undefined;
	
        var tileSize = this.getTileSize();
        tile.setAttribute('width', tileSize.x);
        tile.setAttribute('height', tileSize.y);

        var ctx = tile.getContext('2d');
	//
	var latlng             = tile_to_degree(coords,coords.z,false);
	var nanocubeTileCoords = degree_to_tile(latlng,coords.z,true);
	var nanocubeInterface = this.options.nanocubeInterface;
	var colorScale = this.options.parent.colorScale;
	var layer = this;
	
	//
	if(nanocubeInterface){
	    var nanoCoords = toNanocubeCoords(coords);
	    var resolution = 5;
	    var query = new NanocubeQuery("","TILE",{"tile":nanoCoords,"diveLevel":resolution,"resolution":resolution},"visits","'location'",function(result){
		tile.data = result;
		//		
		var numPixels = 2**resolution;
		var pixelWidth  = tileSize.x / numPixels;
		var pixelHeight = tileSize.y / numPixels;		
		var colors = placeClassColorScale.range();
		if(result.pixels.length > 0){
		    var valueExtent = d3.extent(result.values);
		    layer.updateExtent(valueExtent);
		    var globalExtent = layer.extent;
		    
		    
		    result.pixels.forEach((pixel,index)=>{			
			var value = (result.values[index] - globalExtent[0]) / (globalExtent[1] - globalExtent[0]);
			ctx.fillStyle = colorScale(value);
			ctx.fillRect(pixel[0]*pixelWidth, (numPixels-pixel[1] - 1)*pixelHeight , pixelWidth, pixelHeight);
		    });
		}
		//
		done(null, tile);
	    });
	    nanocubeInterface.query(query);	    
	}
	else{
	    done(null, tile);
	}
	
        return tile;
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


class NanocubeLayer{

    constructor(containerMap, nanocubeInterface, minZoomLevel, maxZoomLevel){
	//
	this.containerMap = containerMap;
	this.nanocube = nanocubeInterface;

	//
	this.minNormalizer = 0.0;
	this.maxNormalizer = 1.0;

	//
	this.colorScale = d3.scaleQuantile().domain([0,1]).range(['#ffffcc','#ffeda0','#fed976','#feb24c','#fd8d3c','#fc4e2a','#e31a1c','#b10026']);
	
	
	//this.colorScale = d3.scaleQuantile().domain([0,1]).range(['#000000', '#060401', '#0b0602', '#100a04', '#140d05', '#171006', '#1a1207', '#1d1509', '#1f160a', '#21180b', '#241a0c', '#271b0d', '#2a1d0e', '#2d1f0f', '#302110', '#322311', '#362512', '#382713', '#3b2913', '#3e2b14', '#412d14', '#432f15', '#463116', '#493317', '#4c3518', '#4f3718', '#533919', '#563b1a', '#583e1b', '#5b401c', '#5e411d', '#62441e', '#65461f', '#684920', '#6b4b21', '#6d4d22', '#704f23', '#745225', '#775426', '#7a5527', '#7d5828', '#805b2a', '#835c2b', '#865f2c', '#8a612e', '#8c632f', '#8f6631', '#936933', '#966b34', '#996d36', '#9c6f37', '#9f723a', '#a2743b', '#a5773d', '#a8793f', '#ab7c42', '#ae7f43', '#b18146', '#b48347', '#b7864a', '#b9894c', '#bd8c4f', '#c08e51', '#c29153', '#c69356', '#c99659', '#cb995b', '#ce9b5f', '#d19f62', '#d3a164', '#d6a467', '#d9a76b', '#dcaa6e', '#deac71', '#e1af74', '#e3b178', '#e6b57c', '#e8b880', '#ebbb84', '#edbd88', '#efc08c', '#f1c390', '#f4c694', '#f6c999', '#f7cc9e', '#f9cfa2', '#fbd2a7', '#fcd6ad', '#fed9b2', '#ffdcb8', '#ffe0be', '#ffe3c5', '#ffe7cc', '#ffebd2', '#ffeeda', '#fff2e1', '#fff5e7', '#fff8f0', '#fffbf7', '#ffffff']);


	
	//
	var that = this;
	this.tileLayer = L.gridLayer.canvasLayer({"nanocubeInterface":nanocubeInterface, "parent":that});
	this.tileLayer.on("load",that.tileLayer.updateTiles);
	this.containerMap.on("zoomstart",function(){
	    that.tileLayer.setExtent([Infinity,-Infinity]);
	});
	this.tileLayer.addTo(this.containerMap);
	//
	this.containerMap.addLayer( L.gridLayer.debugCoords() );

    }
    
    setOpacity(newValue){
	this.tileLayer.setOpacity(newValue);
    }

    renderData(){
	//console.log("Nanocube layer: render");
    }
}
