class TimeSeriesBandPlot{
    
    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight,baseTime){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:50,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;
	this.baseTime = baseTime;
	//
	this.bands  = [];
	this.curves = [];
	//
	this.canvas = container
	    .append("g")
	    .attr("id","line_" + widgetID)
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");
	
	//
	this.xScale = d3.scaleTime().range([0,this.canvasWidth]);
	this.xAxis  = d3.axisBottom(this.xScale);
	this.canvas
	    .append("g")
	    .attr("class","xAxis")
	    .attr("transform","translate(0," + this.canvasHeight  + ")");

	this.canvas.append("g").attr("id",widgetID+"bands");
	this.canvas.append("g").attr("id",widgetID+"lines");
	
	//
	this.yScale = d3.scaleLinear().range([this.canvasHeight,0]);
	this.yAxis  = d3.axisLeft(this.yScale)
	    .tickFormat(d3.format("d"))
	    .ticks(5);
	this.canvas
	    .append("g")
	    .attr("class","yAxis");
	
	//
	var brushGroup = this.canvas.append("g")
	    .attr("class","brushGroup");
	var myBrush = d3.brushX();
	brushGroup.call(myBrush);

	//
	this.canvas.append("text").attr("id",widgetID + "_labelXAxis");
	this.canvas.append("text").attr("id",widgetID + "_labelYAxis");
	this.xLabel = "Time";
	this.yLabel = "";
	//
	this.updatePlot();
    }

    setTimeSelectionCallBack(myCallback,moveCallback){
	var brushGroup = this.canvas.select(".brushGroup");
	var myBrush = d3.brushX();
	var scale = this.xScale;
	var that = this;
	//
	if(moveCallback){
	    myBrush.on("brush", (function(){
		if(d3.event.selection == null)
		    myCallback(null)
		else{
		    var limits = d3.event.selection.map(d=>scale.invert(d));
		    limits[0] = Math.floor(limits[0]/1000);
		    limits[1] = Math.ceil(limits[1]/1000);
		    moveCallback({"widgetID":this.widgetID,"constraints":limits});
		}
	    }).bind(this));
	}
	//
	if(myCallback){
	    myBrush.on("end", (function(){
		//console.log("brush");
		if(d3.event.selection == null)
		    myCallback(null)
		else{
		    var limits = d3.event.selection.map(d=>scale.invert(d));
		    limits[0] = Math.floor(limits[0]/1000);
		    limits[1] = Math.ceil(limits[1]/1000);
		    myCallback({"widgetID":this.widgetID,"constraints":limits});
		}
	    }).bind(this));
	}
	
	brushGroup.call(myBrush);
    }
    
    setData(bands,curves){
	this.curves = curves;
	this.bands = bands;
	this.updatePlot();
    }

    setYLabel(yLabel){
	this.yLabel = yLabel;	
    }
    
    clearPlot(){
	this.setData([],[]);
    }
    
    updateAxis(){
	var canvasWidth = this.canvasWidth;
	var canvasHeight = this.canvasHeight;
	
	//text label for the x axis
	this.xAxis(this.canvas.select(".xAxis"));
	this.canvas.select("#" + this.widgetID + "_labelXAxis")
	    .attr("x",(canvasWidth/2.0))
	    .attr("y",(canvasHeight + this.margins.top + 20))
	    .style("text-anchor", "middle")
	    .text(this.xLabel);

	// this.canvas.select(".xAxis")
	//     .selectAll("text").style("font-size","18px"); //To change the font size of texts

	
	//text label for the y axis
	this.yAxis(this.canvas.select(".yAxis"));
	this.canvas.select("#" + this.widgetID + "_labelYAxis")
	    .attr("transform", "rotate(-90)")
	    .attr("y", 0 - this.margins.left)
	    .attr("x",0 - (canvasHeight / 2))
	    .attr("dy", "1em")
	    .style("text-anchor", "middle")
	    .text(this.yLabel);
    }

    updateBands(){
	var that = this;
	
	var bands = this.canvas.select("#"+this.widgetID+"bands").selectAll(".band")
	    .data(this.bands);

	var valueline = d3.line()
	    .x(function(d) { return that.xScale(d[0]); })
	    .y(function(d) { return that.yScale(d[1]); });

	//
	bands.exit().remove();
	bands.enter()
	    .append("path")
	    .merge(bands)
	    .attr("class","band")
	    .attr("d",valueline)
	    .attr("fill","rgba(255,0,0,0.5)");

	//
	var curves = this.canvas.select("#"+this.widgetID+"lines").selectAll(".curve").data(this.curves);
	curves.exit().remove();

	curves.enter()
	    .append("path")
	    .merge(curves)
	    .attr("class","curve")
	    .attr("d", function(d){
		//console.log("data",d);
		return valueline(d.curve);
	    })
	    .attr("fill","none")
	    .attr("stroke",d=>d.color)
	    .attr("stroke-width",4);
    }
    
    updatePlot(){
	//
	var xExtents = [];
	var yExtents = [];
	this.bands.concat(this.curves.map(d=>d.curve)).forEach(function(band){
	    //
	    var lineXExtent = d3.extent(band,d=>d[0]);
	    xExtents.push(lineXExtent[0]);
	    xExtents.push(lineXExtent[1]);
	    //
	    var lineYExtent = d3.extent(band,d=>d[1]);
	    yExtents.push(lineYExtent[0]);
	    yExtents.push(lineYExtent[1]);
	});

	//
	var xExtent = d3.extent(xExtents);
	//xExtent = [new Date("2017-01-01"), new Date("2017-12-31")]
	//console.log("x extent", xExtent);
	this.xScale.domain( xExtent );
	this.xAxis.scale(this.xScale);
	//
	var yExtent = d3.extent(yExtents);
	if(yExtent[0] == yExtent[1]){
	    yExtent[0] -= (0.1*yExtent[0]);
	    yExtent[1] += (0.1*yExtent[1]);
	}
	this.yScale.domain( yExtent );
	this.yAxis.scale(this.yScale);
	//
	this.updateAxis()
	//points
	this.updateBands()
    }
}
