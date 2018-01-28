var editorProxy = {
    createSelection: function(start, end) {
        if (typeof end === 'undefined') {
				end = start;
        }
        createGeanySelection(start, end);
    },
    getCurrentLineRange: function() {
        return {
            start: getGeanyLineStart(),
            end: getGeanyLineEnd()
        };
    },
    getCaretPos: function() {
        return getGeanyCaretPos();
    },
    setCaretPos: function(pos){
        setGeanyCaretPos(pos);
    },
    getCurrentLine: function() {
        return getGeanyCurrentLine();
    },
    getSelectionRange: function() {
        if(getGeanySelection()){
            return {
                start: getGeanySelectionStart(),
                end: getGeanySelectionEnd()
            };
        }
        var caretPos = this.getCaretPos();
        var d = {
            start: caretPos,
            end: caretPos
        };
        var lineContent = getGeanyLineContent().trim();
        if (!lineContent){
            return d;
        }
        // return d;
        var abbr = emmet.utils.action.extractAbbreviation(lineContent);
        if (!abbr){
            return d;
        }
        if(lineContent === abbr){
            d.start = caretPos - abbr.length;
        }
        else{
            d = this.getCurrentLineRange();
        }
        this.createSelection(d.start, d.end);
        return d;  
    },
    replaceContent: function(value, start, end, noIndent) {
        // value = preProcessText(value);
        value = emmet.utils.editor.normalize(value);
        replaceGeanyContent(value, start);
    },
    getContent: function() {
        return getGeanyContent();
    },
    getSyntax: function() {
        return getGeanySyntax().toLowerCase();
    },
    getProfileName: function() {
        return detectProfile();
    },
    prompt: function(title) {
        return geanyPrompt();
    },
    getSelection: function() {
        return getGeanySelection();
    },
    getFilePath: function() {
        return getGeanyFilePath();
    },
    setIndicator: function(range){
        var start = range.start+1;
        var stop = range.end-1;
        highlightGeanyTag(start, stop);
    }
};

function preProcessText(value) {
	var tabstopOptions = {
		tabstop: function(data){
			var placeholder = data.placeholder;
			if (placeholder) {
				placeholder = emmet.tabStops.processText(placeholder, tabstopOptions);
			}
            return placeholder;

		},
	};
	value = emmet.tabStops.processText(value, tabstopOptions);
	return value;
}

function detectProfile() {
    return emmet.utils.action.detectProfile(editorProxy);
}
function runAction(name){
    var indentation = '\t';
    if (!isGeanyTabUsed()) {
        indentation = emmet.utils.common.repeatString(' ', +getGeanyTabWidth());
    }
    emmet.resources.setVariable('indentation', indentation);
    emmet.utils.common.setCaretPlaceholder('%cursor%');
    return emmet.run(name, editorProxy);
}
function matchPairHighlight(editor) {
    htmlMatcher = emmet.htmlMatcher;
    var content = String(editor.getContent());
	var caretPos = editor.getCaretPos();
	if (content.charAt(caretPos) == '<')
            // looks like caret is outside of tag pair
            caretPos++;
	    var tag = htmlMatcher.tag(content, caretPos);
	    if(!tag) return false;
        if(tag.open.range.inside(caretPos)){
            editor.setIndicator(tag.open.range);
            if(tag.close){
                editor.setIndicator(tag.close.range);
            }
		}
		else if(tag.close && tag.close.range.inside(caretPos)){
            editor.setIndicator(tag.open.range);
            editor.setIndicator(tag.close.range);
		}
		return true;
}
emmet.actions.add('highlight_tag', matchPairHighlight, {hidden: true});
