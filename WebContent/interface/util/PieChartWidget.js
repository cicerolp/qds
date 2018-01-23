class PieChartWidget{

    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight,baseTime){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:50,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;
	this.dataInitialized = false;
	//
	this.canvas = container
	    .append("g")
	    .attr("id","piechart_" + widgetID);
	this.canvasPieChart = this.canvas.append("g")	
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left+this.canvasWidth/2) + ", " + (this.renderingArea.y+this.margins.top+this.canvasHeight/2) + ")");
	this.canvasPieChart.append("g").attr("class","slices");
	this.canvasPieChart.append("g").attr("class","labels");
	//
	this.colorScale = d3.scaleOrdinal(d3.schemePaired);
	this.data = [];
	this.selected = new Set([]);
	this.selectionChangedCallback = undefined;
	this.maxCategories = 4;
	//
	this.canvas.append("text").attr("class","title").attr("x",this.canvasWidth/2).attr("y",this.margins.top+2) ;
    }

    setData(newData){	
	this.dataInitialized = true;
	this.updatePlot();
	if(newData.length > this.maxCategories){
	    //sort values and get the this.maxCategories first (largest) ones
	    newData = newData.sort(function (a, b) {
		if (a.numVisits < b.numVisits) {
		    return 1;
		}
		if (a.numVisits > b.numVisits) {
		    return -1;
		}
		// a must be equal to b
		return 0;
	    });
	    //
	    var others = 0;
	    for(var i = this.maxCategories ; i < newData.length ; ++i){
		others += newData[i].numVisits;
	    }
	    //
	    this.data = newData.slice(0,this.maxCategories);
	    this.data.push({"appName":"others","numVisits":others});
	    console.log(this.data);
	}
	else
	    this.data = newData;
	//
	this.updatePlot();
    }

    setSelectionChangedCallback(f){
	this.selectionChangedCallback = f;
    }
    
    updatePlot(){
	if(!this.dataInitialized)
	    return;

	var widget = this;

	this.canvas.select(".title")
	    .text("App Visits");
	
	if(true){
	    var pie = d3.pie()
		.sort(null)
		.value(function(d) { return d.numVisits; });

	    var radius = d3.min([this.canvasWidth/2,this.canvasHeight/2]);
	    
	    var path = d3.arc()
		.outerRadius(radius * 0.8)
		.innerRadius(0);

	    var label = d3.arc()
		.outerRadius(radius * 0.9)
		.innerRadius(radius * 0.9);

	    //
	    this.canvasPieChart.selectAll("path").remove();
	    this.canvasPieChart.selectAll("text").remove();
	    
	    //
	    var arc = this.canvasPieChart.select(".slices")
		.selectAll("path")
		.data(pie(this.data)).enter();

	    arc.append("path")
		.attr("d", path)
		.attr("fill", (function(d) { return this.colorScale(d.data.appName); }).bind(this))
		.on("click",function(d){		
		    var thisSelection = d3.select(this);
		    var x = thisSelection.attr("stroke");
		    if(x == null){
			thisSelection.attr("stroke-width",3);
			thisSelection.attr("stroke","black");
			widget.selected.add(d.data.appName);
		    }
		    else{
			thisSelection.attr("stroke",null);
			widget.selected.delete(d.data.appName);
		    }
		    //
		    if(widget.selectionChangedCallback){
			widget.selectionChangedCallback({"widgetID":widget.widgetID,"constraints":widget.selected});
		    }		
		});

	    //
	    arc = this.canvasPieChart.select(".labels").selectAll("text").data(pie(this.data)).enter();
	    arc.append("text")
	    	.attr("transform", function(d) { return "translate(" + label.centroid(d) + ")"; })
	    	.attr("dy", "0.35em")
	    	.text(function(d) {
	    	    return d.data.appRealName; });	    
	}
	else{
	    var pie = d3.pie()
		.sort(null)
		.value(function(d) { return d.numVisits; });

	    var radius = d3.min([this.canvasWidth/2,this.canvasHeight/2]);
	    
	    var arc = d3.arc()
		.outerRadius(radius * 0.8)
		.innerRadius(radius * 0.4);
	    
	    var outerArc = d3.arc()
		.innerRadius(radius * 0.9)
		.outerRadius(radius * 0.9);

	    var svg = this.canvas;
	   
	    
	    /* ------- PIE SLICES -------*/
	    var key = function(d){ return d.data.appName; };

	    var slice = svg.select(".slices").selectAll("path")
		.data(pie(this.data), key);

	    slice.enter()
		.append("path")
		.style("fill", function(d) { return widget.colorScale(d.data.label); })
		.attr("class", "slice")
		.attr("d", arc);

	    slice		
		.attr("d", arc);

	    slice.exit()
		.remove();
	    

	    /* ------- TEXT LABELS -------*/

	    var text = svg.select(".labels").selectAll("text")
		.data(pie(this.data), key);

	    
	    function midAngle(d){
		return d.startAngle + (d.endAngle - d.startAngle)/2;
	    }

	    text.enter()
		.append("text")
		.merge(text)
		.attr("dy", ".35em")
		.text(function(d) {
		    return d.data.appName;
		})
		.attr("transform", function(d2) {
		debugger
		    var pos = outerArc.centroid(d2);
			pos[0] = radius * (midAngle(d2) < Math.PI ? 1 : -1);
			return "translate("+ pos +")";
		})
		.style("text-anchor", function(d2){
		    var d2 = interpolate(t);
		    return midAngle(d2) < Math.PI ? "start":"end";
		});

	    text.exit()
		.remove();

	    /* ------- SLICE TO TEXT POLYLINES -------*/

	    var polyline = svg.select(".lines").selectAll("polyline")
		.data(pie(this.data), key);
	    
	    polyline.enter()
		.append("polyline");

	    polyline.transition().duration(1000)
		.attrTween("points", function(d){
		    this._current = this._current || d;
		    var interpolate = d3.interpolate(this._current, d);
		    this._current = interpolate(0);
		    return function(t) {
			var d2 = interpolate(t);
			var pos = outerArc.centroid(d2);
			pos[0] = radius * 0.95 * (midAngle(d2) < Math.PI ? 1 : -1);
			return [arc.centroid(d2), outerArc.centroid(d2), pos];
		    };			
		});
	    
	    polyline.exit()
		.remove();
	}
    }
}
