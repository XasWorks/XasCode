
// Currently used supervariables are: $shown, $not-shown, $colortable, $highlighted, $opacity

function contains(sArray, cString) = (len(sArray) > 0) ? max( [for(i = [0:len(sArray) -1]) sArray[i] == cString ? 1 : 0 ]) == 1 : false;

module color_appropriately(tagname) {
	if(!contains($colortable, tagname)) {
		children();
	}
	else {
		coloring = $colortable[search([tagname], $colortable, 1, 0)[0] + 1];
		color(coloring, $opacity) children();
	}
}

module separate_tagging() {
	$shown = 0;
	$not_shown = 0;
	children();
}

// The tagging function, the core of the system.
// It only shows a module under following circumstances:
// -- It is listed in the $shown array
// -- The $show array is empty and it is not excluded (via $not_shown) AND it is a foreground object
module tag(tagname, foreground = true) {
	color_appropriately(tagname)
		if(len($shown) > 0) {
			if(contains($shown, tagname))
				separate_tagging() render(convexity = 5) children();
		}
		else if(foreground && !contains($not_shown, tagname))
			separate_tagging() render(convexity = 5) children();

	if(contains($highlighted, tagname)) {
		$opacity = 0.2;
		%color_appropriately(tagname)
		separate_tagging() render(convexity = 5) children();
	}
}

module showOnly(tagname) {
	$shown = concat(tagname);
	children();
}

module showTag(tagname) {
	if(!(contains($shown, tagname))) {
		$shown = concat($shown, tagname);
		children();
	}
	else
		children();
}

module hideTag(tagname) {
	$managed = true;
	if(!(contains($not_shown, tagname) || contains($shown, tagname))) {
		$not_shown = concat($not_shown, tagname);
		children();
	}
	else
		children();
}

module colorTag(tagname, coloring) {
	$colortable = concat($colortable, [tagname, coloring]);
	children();
}

module highlightTag(tagname) {
	$highlighted = concat($highlighted, tagname);

	children();
}

module taggedUnion(targets, tagname, foreground = true) {
	union() {
		tag(tagname, foreground)
		union() {
			showTag(targets) children();
		}

		if(foreground) {
			hideTag(targets) children();
		}
		else {
			children();
		}
	}
}

module taggedDifference(positives, negatives, tagname, foreground = true) {
	union() {
		tag(tagname, foreground)
		difference() {
			showTag(positives) children();
			showTag(negatives) children();
		}

		if(foreground) {
			hideTag(concat(positives, negatives)) children();
		}
		else {
			children();
		}
	}
}

module taggedIntersection(targets, tagname, foreground = true) {
	union() {
		tag(tagname, foreground)
		intersection_for(i = targets) {
			showTag(i) children();
		}

		if(foreground) {
			hideTag(targets) children();
		}
		else {
			children();
		}
	}
}
