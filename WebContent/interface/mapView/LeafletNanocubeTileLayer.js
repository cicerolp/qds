 class LeafletNanocubeTileLayer {

    constructor(container){
	this.map   = container;
	this.layer = L.geoJson();
	this.layer.addTo(this.map);
	this.enabled = true;	
    }

    setEnabled(v){

	if(!this.enabled && v){
	    this.map.addLayer(this.layer);
	}
	else if(!v){
	    this.map.removeLayer(this.layer);
	}
	this.enabled = v;
    }
    
    clearData(){
	this.layer.clearLayers();
    }
    
    setData(newData){
	this.layer.addData(countriesData);
    }

    setColormapFunction(){
    }

    setStyle(f){
	var styleFunction = function(feature){
	    return {
		fillColor: f(feature.properties.numPlaces),
		weight: 2,
		opacity: 1,
		color: 'white',
		dashArray: '3',
		fillOpacity: 0.7
	    };
	}

	this.layer.setStyle(styleFunction);
    }
    
    renderData(){
    }
    
}
