class GLLeafletMap{

    constructor(containerID, defaultCoords, defaultZoomLevel, tileURL, tileLayerProperties){
	this.containerID = containerID;
	this.map = L.map(containerID,{renderer: L.canvas()}).setView(defaultCoords, defaultZoomLevel);
	this.tileLayer = L.tileLayer(tileURL, tileLayerProperties);
	this.tileLayer.addTo(this.map);

	//
	this.glLayer = L.canvasOverlay()
            .drawing(this.renderLayers.bind(this))
            .addTo(this.map);

	//
        this.canvas = this.glLayer.canvas();
        this.glLayer.canvas.width = this.canvas.clientWidth;
        this.glLayer.canvas.height = this.canvas.clientHeight;
        this.glContext = this.canvas.getContext('experimental-webgl', { antialias: true });
	this.shaderManager = new ShaderManager(this.glContext);
	this.layerManager = new LayerManager();
	this.glLayer.redraw();

	//
	this.callbacks = [];
	//
	this.addControls();

	//
	this.setupInteraction();
	
    }

    setTileLayerOpacity(v){
	this.tileLayer.setOpacity(v);
    }
    
    setCallback(eventName,callback){
	this.callbacks[eventName] = callback;
    }

    emitSelectionChanged(){
	if('selectionChanged' in this.callbacks){
	    this.callbacks['selectionChanged']({"widgetID":this.containerID,"constraints":this.brushManager.getSelectionBounds()});
	}
    }
    
    setupInteraction(){
	//SET UP DEFAULT
	this.brushManager = new BrushManager(this.map);
	this.initialPoint = [0,0];
	this.lastPoint = [0,0];
	this.interactionFlag = 0; //IDLE
	this.currentSelection = undefined;//L.rectangle(bounds, {color: 'blue', weight: 1}).addTo(this.map);		
	
	//END INTERACTION
	this.map.on('mouseup', (function(){
	    if(this.interactionFlag == 1 || this.interactionFlag == 2){
		this.emitSelectionChanged();
	    }
	    this.interactionFlag = 0;
	    this.currentSelection = undefined;
	    this.map.dragging.enable();
	}).bind(this));
    
	//START INTERACTION
	this.map.on('mousedown', (function (e){
	    //e.originalEvent.preventDefault();
	    //e.originalEvent.stopPropagation();

	    //debugger
	    if(e.originalEvent.button == 2){
	    	this.map.dragging.disable();
	    	this.interactionFlag = 1;//drawing selection
		//
		this.initialPoint = [e.latlng.lng,e.latlng.lat];
		this.lastPoint = [e.latlng.lng+0.0001,e.latlng.lat+0.0001];
		this.currentSelection = this.brushManager.getFirstThatContains(e.latlng);
		if(this.currentSelection){
		    if(e.originalEvent.ctrlKey){
			this.brushManager.remove(this.currentSelection.internalID);
			this.interactionFlag = 0;
			this.emitSelectionChanged();
		    }
		    else{
			this.interactionFlag = 2;
		    }
		}
		else{
		    this.currentSelection = undefined;
		    this.interactionFlag = 1;
		}
	    }	
	}).bind(this));
	
	//MOVE INTERACTION
	this.map.on('mousemove', (function(e){
	    
	    if(this.interactionFlag == 1){
		if(this.currentSelection == undefined){
		    if(this.brushManager.getNumBrushes() == 1){
			var existingBrush = this.brushManager.brushes[0];
			this.brushManager.remove(existingBrush.internalID);
		    }
		    this.currentSelection = this.brushManager.newBrush();
		}

		this.lastPoint = [e.latlng.lng,e.latlng.lat];
		var lngs = d3.extent([this.initialPoint[0],this.lastPoint[0]]);
		var lats = d3.extent([this.initialPoint[1],this.lastPoint[1]]);
		var currentSelectionBounds = [[lats[0],lngs[0]],[lats[1],lngs[1]]];
		this.currentSelection.setBounds(currentSelectionBounds);
	    }
	    else if(this.interactionFlag == 2){
		var delta = [e.latlng.lng - this.lastPoint[0],e.latlng.lat - this.lastPoint[1]]
		//
		var aux = this.currentSelection.getLatLngs();
		aux = Array.isArray(aux)?aux[0]:aux;//HACK beacuse it seems to return distinct things in different machines check this
		//
		aux = aux.map(function(d){
		    d.lat += delta[1];
		    d.lng += delta[0];
		    return d;
		});
		
		this.currentSelection.setLatLngs(aux);		
	    }
	    this.lastPoint = [e.latlng.lng,e.latlng.lat];
	}).bind(this));
	
	//AVOID MENU WHEN RIGHT CLICK
	this.map.addEventListener('contextmenu', function(evt) { 
	    evt.originalEvent.preventDefault();
	}, false);
	
    }
    
    addControls(){
	var myMap = this.map;

	/**************
         * add Legend *
         **************/

	//
	var legendSeq = L.control({position: 'bottomleft'});
	legendSeq.onAdd = function (map) {

	    var div = L.DomUtil.create('div', 'info legendSVG');

	    div.innerHTML = "<svg id=\"legendSeq\" width=\"130\" height=\"180\"><text class=\"title\"  font-weight=\"bold\" x=\"20\" y=\"15\">Title</text><g class=\"legendQuant\" transform=\"translate(20,25)\"></g></svg>"
	    
	    return div;
	};
	legendSeq.addTo(myMap);
	
	//
	var legend = L.control({position: 'bottomleft'});
	legend.onAdd = function (map) {

	    var div = L.DomUtil.create('div', 'info legendSVG');

	    div.innerHTML = "<svg id=\"legend\" width=\"130\" height=\"180\"><text class=\"title\"  font-weight=\"bold\" x=\"20\" y=\"15\">Title</text><g class=\"legendQuant\" transform=\"translate(20,25)\"></g><g class=\"sliderAxis\" transform=\"translate(80,25)\"></g><g class=\"scentedRectsGroup\" transform=\"translate(70,25)\"></g><g class=\"brushGroup\" transform=\"translate(70,25)\"></g></svg>"
	    
	    return div;
	};
	legend.addTo(myMap);
		
	/**************************
         * add dive level control *
         **************************/

	var parameterControl = L.control({position: 'bottomright'});
	parameterControl.onAdd = function (map) {

	    var div = L.DomUtil.create('div', 'info controls');
	    div.setAttribute("id","heatmapControls");
	    
	    div.innerHTML = "<b> Heatmap Level:</b></br><input type=\"range\" id=\"geoLevelSlider\" min=\"0\" max=\"10\" step=\"1\" value=\"5\"></br><b> Heatmap Opacity:</b></br><input type=\"range\" id=\"heatmapOppacitySlider\" min=\"0\" max=\"255\" step=\"1\" value=\"255\">"
	    
	    return div;
	};
	parameterControl.addTo(this.map);


	/**************************
         * add layers control *
         **************************/
	
	var layersControl = L.control();
	layersControl.onAdd = function (map) {

	    var div = L.DomUtil.create('div', 'controls');
	    div.setAttribute("id","MyLayersControl");
	    div.innerHTML = "<b> Enable Layer:</b></br><div id=\"layerChecks\"></div>"
	    return div;
	};
	layersControl.addTo(this.map);

	/******************************************************************
         * control interaction so click on controls do not affect the map *
         ******************************************************************/
	
	var controls = [legend, legendSeq, parameterControl,layersControl];
	controls.forEach(function(d){
	    //turn of events when mouse on legend
	    d.getContainer().addEventListener('mouseover', function () {
		myMap.dragging.disable();
		myMap.doubleClickZoom.disable();
	    });
	    //set on again
	    d.getContainer().addEventListener('mouseout', function () {
		myMap.dragging.enable();
		myMap.doubleClickZoom.enable();
	    });
	});

    }
    
    getLayer(layerName){
	return this.layerManager.getLayer(layerName);
    }
    
    updateLegend(colorScale,scaleType,legendTitle,legendID, minVal, maxVal, payload){

	//hack weird to remove weird svg that is being added artificially
	var ySlack = 2;
	var xSlack = 3;

	//
	var svg = d3.select(legendID);
	svg.selectAll("svg").remove();
	svg.select(".title")
	    .text(legendTitle);

	//
	var group = svg.select(".legendQuant");
	//debugger
	if (scaleType == "continuousLog") {

	    var rectHeight = 130;
	    var rectWidth = 10;

	    var defs = svg.append("defs");
	    var linearGradient = defs.append("linearGradient")
		.attr("id", "count-gradient");
	    
	    linearGradient
		.attr("x1", "0%")
		.attr("y1", "100%")
		.attr("x2", "0%")
		.attr("y2", "0%");

	    linearGradient.selectAll("stop")
		.data(colorScale.range())
		.enter()
		.append("stop")
		.attr("offset", function(d, i) { return (i/(colorScale.range().length - 1)); })
		.attr("stop-color", d => d);
	    
	    //
	    //svg.attr("height", function () { return 40 + (colorScale.domain().length) * (rectHeight + ySlack) });
	    //
	    group.selectAll("rect")
		.remove();

	    group
		.append("rect")
		.attr("fill", "url(#count-gradient)")
		.attr("y", function (d, i) { return (rectHeight + ySlack) * i; })
		.attr("width", rectWidth)
		.attr("height", rectHeight)
		.style("stroke-width", "1")
		.style("stroke", "black");
	    //

	    var yScale = d3.scaleLinear()
		.domain(colorScale.domain().reverse())
		.range([0,130]);

	    var yAxis = d3.axisLeft()
		.scale(yScale);
		// .tickSize(10)
		// .tickSizeInner(5)
		// .ticks(4, ".0s");

	    group.selectAll("g")
		.remove();

	    var axisGroup = group.append("g")
		.call(yAxis)
		.attr("transform", "translate("+(30+rectWidth)+",0)");

	    // axisGroup.selectAll(".tick")
	    // 	.select("text")
	    // 	.attr("transform", "translate(5,0)");


	    var myBrush = d3.brushY().extent([[0, 0], [rectWidth+xSlack, rectHeight]]);
	    myBrush.on("brush", (function () {
		var countExtent = d3.event.selection.map(d => yScale.invert(d));
		console.log("brush!");
		this.getLayer("ndsLayer").setInverseQuantileFilter(countExtent[1],countExtent[0]);
	    }).bind(this))
		.on("end", function () {
		    if (d3.event.selection == null)
			svg.select(".brushGroup").call(myBrush.move, [0, rectHeight])
		});
	    svg.select(".brushGroup").call(myBrush).call(myBrush.move, [0, rectHeight]);
	}
	else if(scaleType == "categorical"){
	    var rectHeight = 14;
	    var rectWidth = 15;
	    //
	    svg.attr("height",function(){return 40+(colorScale.domain().length)*(rectHeight+ySlack)});
	    //
	    var rects = group.selectAll("rect").data(colorScale.range());
	    rects.exit().remove();
	    rects.enter()
		.append("rect")
		.merge(rects)
		.attr("y",function(d,i){return (rectHeight + ySlack)*i;})
		.attr("width",rectWidth)
		.attr("height",rectHeight)
		.attr("fill",d=>d)
	    	.style("stroke-width","1")
		.style("stroke","black");

	    //
	    var labels = group.selectAll("text").data(colorScale.domain());
	    labels.exit().remove();
	    labels.enter()
		.append("text")
		.merge(labels)
		.attr("y",function(d,i){return 0.6*rectHeight+(rectHeight + ySlack)*i})
		.attr("x",function(){return (xSlack + rectWidth)})
		.attr("alignment-baseline","middle")
		.text(d=>d);
	}
	else if(scaleType == "sequential"){
	    //in this case payload means scented histogram
	    
	    //
	    var rectHeight = 15;
	    var rectWidth = 15;
	    //
	    svg.attr("height",function(){return 40+(colorScale.domain().length)*(rectHeight+ySlack)});
	    //
	    var numRects = colorScale.range().length;
	    var legendRects = group.selectAll(".legendRects").data(colorScale.range());
	    legendRects.exit().remove();
	    legendRects.enter()
		.append("rect")
		.merge(legendRects)
		.attr("y",function(d,i){return (rectHeight + ySlack)*i;})
		.attr("width",rectWidth)
		.attr("height",rectHeight)
		.attr("fill",d=>d)
		.attr("class","legendRects")
		.style("stroke-width","1")
		.style("stroke","black");
	    //
	    var maximumScreenCoord = (rectHeight+ySlack)*numRects - ySlack;
	    var axisScale = d3.scaleLinear().domain([minVal,maxVal]).range([0, maximumScreenCoord]);
	    var myAxis = d3.axisLeft(axisScale).tickFormat(d3.format(".2s"));
	    svg.select(".sliderAxis").call(myAxis);

	    //
	    if(payload){
		var xScale = d3.scaleLinear().domain(d3.extent(payload)).range([0,40]);
		var barHeight = maximumScreenCoord / payload.length;
		var scentedGroup = svg.select(".scentedRectsGroup");
		var scentedRects = scentedGroup.selectAll(".scentedRect").data(payload);
		scentedRects.exit().remove();
		scentedRects.enter()
	    	    .append("rect")
	    	    .merge(scentedRects)
	    	    .attr("class","scentedRect")
		    .attr("x",10)
	    	    .attr("y",function(d,i){return i*barHeight;})
	    	    .attr("width",d=>Math.ceil(xScale(d)))
	    	    .attr("height",barHeight)
	    	    .attr("fill","red");
	    }
	    
	    //
	    var myBrush = d3.brushY().extent([[0, 0], [40, maximumScreenCoord]]);
	    myBrush.on("brush",(function(){
		var countExtent = d3.event.selection.map(d=>axisScale.invert(d));
		//this.getLayer("Visits Layer").setColorNormalization(countExtent[0],countExtent[1]);
		this.repaint();
	    }).bind(this))
		.on("end",function(){
		if(d3.event.selection == null)
		    svg.select(".brushGroup").call(myBrush.move,[0,maximumScreenCoord])
	    });
	    svg.select(".brushGroup").call(myBrush).call(myBrush.move,[0,maximumScreenCoord]);
	    
	    /*var rectHeight = 15;
	    var rectWidth = 15;
	    //
	    var myDomain              = colorScale.domain(); 
	    var numStops              = myDomain.length;
	    var numColorsBetweenStops = 7;
	    var numRectangles         = (numStops-1)*numColorsBetweenStops;
	    var samplingPoints        = [];

	    //
	    samplingPoints.push(myDomain[0]);
	    for(var intervalIndex = 0; intervalIndex < (numStops-1) ; intervalIndex++){
		var x0 = myDomain[intervalIndex];
		var x1 = myDomain[intervalIndex+1];
		var delta = (x1-x0)/numColorsBetweenStops;
		
		for(var lambda = delta ; lambda <= 1.0 ; lambda+=delta){
		    samplingPoints.push(x0 + lambda*(x1-x0));
		}
	    }
	    
	    //
	    var svgHeight = 40+(numRectangles)*(rectHeight+ySlack);
	    svg.attr("height",function(){return svgHeight;});
	    //
	    var rects = group.selectAll("rect").data(samplingPoints);
	    rects.exit().remove();
	    rects.enter()
		.append("rect")
		.merge(rects)
		.attr("y",function(d,i){return (rectHeight + ySlack)*i;})
		.attr("width",rectWidth)
		.attr("height",rectHeight)
		.attr("fill",d=>colorScale(d));
	    //
	    var myAxis = d3.axisLeft(d3.scaleLinear().range([ySlack, (rectHeight+ySlack)*samplingPoints.length-10]));
	    svg.select(".sliderAxis").call(myAxis);*/
	}
	else{
	    //sequential or diverging
	    
	}
	
	// var colorLegend = d3.legendColor()
	//     .labelFormat(d3.format(".2f"))
	//     .useClass(true)
	//     .scale(quantize);

	// svg.select(".legendQuant")
	//     .call(colorLegend);
	
	// var mySVG = d3.select("#legend");
	// mySVG.selectAll("svg").remove();
	// mySVG.append("rect")
	//     .attr("x",0)
	//     .attr("y",0)
	//     .attr("width",20)
	//     .attr("fill","red")
	//     .attr("height",20);
	
	// var grades = [0, temp, temp * 2, temp * 3, temp * 4, temp * 5];

	// var color = d3.scaleLinear()
	// 	.range([0,1])
	// 	.domain([0, 100]);
	// var temp = color.domain()[1] - color.domain()[0];
	// temp = temp/5;

	// var legendElements = mySVG.selectAll("i").data(grades);
	// legendElements.exit().remove();


	
	// legendElements.enter()
	//     .append("i")
	//     .merge(legendElements)
	//     .attr("style",d=> "background:" + "rgb(255,0,0)");
	
    }

    addLayer(layer,layerID){
	this.layerManager.addLayer(layer,layerID);
	this.updateLayerControls();
    }

    addLeafletGeoJSONLayer(layerID){
	var layer = new LeafletGEOJSONLayer(this.map);
	//only update if data has been passed to this function
	this.layerManager.addLayer(layer,layerID);
	//
	this.updateLayerControls();
	return layer;
    }
    
    addPointLayer(layerID, dataInfo, vertexCoords, colorCoords, minZoom, maxZoom){
	var myPointLayer = new LatLngPointLayer(this.glContext,this.shaderManager,minZoom,maxZoom);
	//only update if data has been passed to this function
	if(dataInfo && vertexCoords && colorCoords){
	    myPointLayer.setData(dataInfo,vertexCoords, colorCoords);
	}
	this.layerManager.addLayer(myPointLayer,layerID);
	//
	this.updateLayerControls();
	return myPointLayer;
    }

    addPolygonLayer(layerID, dataInfo, vertexCoords, faceIndices, minZoom, maxZoom){
	var myPolygonLayer = new LatLngPolygonLayer(this.glContext,this.shaderManager,minZoom,maxZoom);
	//only update if data has been passed to this function
	if(dataInfo && vertexCoords && faceIndices){
	    //
	    myPolygonLayer.setData(dataInfo,vertexCoords, faceIndices);
	}
	this.layerManager.addLayer(myPolygonLayer,layerID);
	//
	this.updateLayerControls();
	return myPolygonLayer;
    }

    addTrajectoryLayer(layerID, minZoom, maxZoom){
	var myTrajectoryLayer = new LatLngTrajectoryLayer(this.glContext,this.shaderManager,minZoom,maxZoom);
	//only update if data has been passed to this function
	this.layerManager.addLayer(myTrajectoryLayer,layerID);
	//
	this.updateLayerControls();
	return myTrajectoryLayer;
    }
    
    addNanocubeLayer(layerID, nanocube, minZoom, maxZoom){
	var myPolygonLayer = new NanocubeLayer(this.map,nanocube,minZoom,maxZoom);
	this.layerManager.addLayer(myPolygonLayer,layerID);
	this.updateLayerControls();
	return myPolygonLayer;
    }
    
    updateLayerControls(){
	//
	var layerNames = this.layerManager.getLayerNames();
	//
	var container = d3.select("#layerChecks");
	var inputs = container.selectAll("input")
	    .data(layerNames);
	//
	inputs.exit().remove();
	//
	var enterSelection = inputs.enter();
	    enterSelection.append("input")
	    .attr("type","checkbox")
	    .attr("id",d=>("enabledCheckBox_"+d))
	    .attr("checked","true")
	    .on("change",(function(d){
		this.setEnabled(d,d3.event.target.checked);
	    }).bind(this));
	enterSelection.append("span").text(d=>d);
	enterSelection.append("br");
    }

    setEnabled(layerName, isEnabled){
	
	d3.select("#layerChecks").select("#enabledCheckBox_" + layerName).attr("checked",isEnabled?true:null);
	this.layerManager.getLayer(layerName).setEnabled(isEnabled);

	//TODO: HARD CODED THIS FOR NOW
	//hide/show legend
	if(layerName == "Places Layer"){
	    if(isEnabled){
		d3.select("#legendSeq").style("display", "block");
	    }
	    else{
		d3.select("#legendSeq").style("display", "none");
	    }
	}
	else if(layerName == "Visits Layer"){
	    if(isEnabled){
		d3.select("#legend").style("display", "block");
	    }
	    else{
		d3.select("#legend").style("display", "none");
	    }
	}
	//
	this.repaint();
    }
    
    latLongToPixelXY(latitude, longitude) {
            var pi_180 = Math.PI / 180.0;
            var pi_4 = Math.PI * 4;
            var sinLatitude = Math.sin(latitude * pi_180);
            var pixelY = (0.5 - Math.log((1 + sinLatitude) / (1 - sinLatitude)) / (pi_4)) * 256;
            var pixelX = ((longitude + 180) / 360) * 256;

            var pixel = { x: pixelX, y: pixelY };

            return pixel;
    }

    repaint(){
	this.glLayer.redraw();
    }

    render(){
	if(this.glContext){
	    //this.glContext.clearColor(1.0, 0.0, 0.0, 1.0);  // Clear to black, fully opaque
	    //gl.viewport(0, 0, canvas.width, canvas.height);
	    this.glContext.clear(this.glContext.COLOR_BUFFER_BIT);

	    //setup rendering
	    // Create matrix
	    var bounds = this.map.getBounds();
            var topLeft = new L.LatLng(bounds.getNorth(), bounds.getWest());
            var offset = this.latLongToPixelXY(topLeft.lat, topLeft.lng);
	    var scale = Math.pow(2, this.map.getZoom());
            
	    var projectionMatrix = mat4.create();
	    mat4.set(projectionMatrix, 2 / this.canvas.width, 0, 0, 0, 0, -2 / this.canvas.height, 0, 0, 0, 0, 0, 0, -1, 1, 0, 1);
	    mat4.scale(projectionMatrix,projectionMatrix,[scale,scale,1.0]);
	    mat4.translate(projectionMatrix, projectionMatrix, [-offset.x, -offset.y,0]);
	    
	    //
	    var usedShaders = ["latLngPointShader","latLngShader","latLngPickingPointShader"];
	    usedShaders.forEach((function(d){
		var myShader = this.shaderManager.getShader(d);
		myShader.bind();
		myShader.setUniformMatrix4fv("uProjectionMatrix",projectionMatrix);
	    }).bind(this));    
	    
	    this.layerManager.render();
	}
    }
    
    renderLayers(canvasOverlay, params){
	this.render();
    }
}
