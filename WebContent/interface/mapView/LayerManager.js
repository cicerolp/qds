class LayerManager{

    constructor(){
	this.layers = {};
    }

    numLayers(){
	return Object.keys(this.layers).length;
    }

    getLayerNames(){
	return Object.keys(this.layers);
    }
    
    getLayer(name){
	if(name in this.layers){
	    return this.layers[name];
	}
	else{
	    alert("layer "+ name + " does not exist!");
	    return undefined;
	}
    }
    
    addLayer(layer,layerID){
	if(layerID in this.layers){
	    alert("ERROR(Layer Manager): Layer " + layerID + " already exists!");
	    return undefined;
	}

	this.layers[layerID] = layer;
    }
    
    render(){
	for(var id in this.layers){
	    this.layers[id].renderData();
	}
    }
}
