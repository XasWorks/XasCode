
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
	color_appropriately(tagname) {
		// If the "shown" array has any entries, ONLY the "shown" entries should be displayed.
		if(len($shown) > 0) {
			if(contains($shown, tagname)) {
				separate_tagging() children();
			}
		}
		else if(foreground && !contains($not_shown, tagname)) {
			separate_tagging() children();
		}
	}


	if(contains($highlighted, tagname) && !contains($shown, tagname)) {
		$opacity = 0.2;	// The "opacity" tag is shared down the CSG tree, also making any children opaqueco
		%color_appropriately(tagname)
			separate_tagging() children();
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

		hideTag((foreground) ? targets : "") children();
	}
}

module taggedDifference(positives, negatives, tagname, foreground = true) {
	union() {
		tag(tagname, foreground)
		difference() {
			showTag(positives) children();
			showTag(negatives) children();
		}

		hideTag((foreground) ? concat(positives, negatives) : "") children();
	}
}

module taggedIntersection(targets, tagname, foreground = true) {
	union() {
		tag(tagname, foreground)
		intersection_for(i = targets) {
			showTag(i) children();
		}

		hideTag((foreground) ? targets : "") children();
	}
}

colorTag("positive", "blue")
taggedDifference("positive", "negative", "neutral", true) {
	tag("positive") sphere(r=10);
	tag("negative") cube(15, true);
}
