class TimeSelectionWidget{
    constructor(container,widgetID,screenX,screenY,totalWidth,totalHeight){
	//set margins
	this.renderingArea = {x:screenX,y:screenY,width:totalWidth,height:totalHeight};
	this.margins = {left:25,right:10,top:10,bottom:30}
	this.canvasWidth = this.renderingArea.width - this.margins.left - this.margins.right;
	this.canvasHeight = this.renderingArea.height - this.margins.top - this.margins.bottom;
	this.widgetID = widgetID;

	//
	this.canvas = container
	    .append("g")
	    .attr("id","line_" + widgetID)
	    .attr("transform","translate("+(this.renderingArea.x+this.margins.left) + ", " + (this.renderingArea.y+this.margins.top) + ")");

	//
	this.selectionChangedCallback = undefined;
	
	//
	this.renderWidget();
    }

    setTimeSelectionChangedCallback(f){
	this.selectionChangedCallback = f;
    }

    renderWidget(){
	//month line
	const months     = ["Jan","Feb","Mar","Apr","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];
	const daysOfWeek = ["Mon","Tue","Wed","Thu","Fri","Sat","Sun"];
	const hours      = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23];
	var lines        = [months,daysOfWeek,hours];
	var numLines     = 3;
	
	//
	var xSlack       = 0;
	var ySlack       = 0;
	const lineWidth  = this.canvasWidth - 2*xSlack;
	const lineHeight = (this.canvasHeight - 2*ySlack)/numLines;

	//
	var lines =
	    this.canvas
	    .selectAll(".lines")
	    .data(lines)
	    .enter()
	    .append("g")
	    .attr("class","lines");

	var cells =
	    lines.selectAll(".cells")
	    .data(function(d,i){return d.map(x=>[x,i,d.length]);});

	cells.exit().remove();

	//
	cells.enter()
	    .append("rect")
	    .attr("x",function(d,i){return xSlack+i*((lineWidth/d[2]));})
	    .attr("y",function(d,i){return d[1]*(ySlack+lineHeight);})
	    .attr("width",function(d,i){return (lineWidth/d[2]);})
	    .attr("height",lineHeight)
	    .style("stroke-width",0.5)
	    .style("stroke","black")
	    .attr("class","cell")
	    .on("click",function(){
		var rect = d3.select(d3.event.target);
		if(rect.attr("class") == "cell"){
		    rect.attr("class","selectedCell");
		}
		else{
		    rect.attr("class","cell");
		}		
	    });

	//
	cells.enter()
	    .append("text")
	    .attr("x",function(d,i){return xSlack+(lineWidth/d[2])/2+i*((lineWidth/d[2]));})
	    .attr("y",function(d,i){return d[1]*(ySlack+lineHeight) + lineHeight/2;})
	    .attr("text-anchor","middle")
	    .attr("alignment-baseline","middle")
	    .style("pointer-events","none")
	    .style("user-select","none")
	    .text(d=>d[0]);
    }

    //
    getTimeFilters(){
	
    }
}
