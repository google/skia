package org.skia.viewer;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.Spinner;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

public class StateAdapter extends BaseAdapter implements AdapterView.OnItemSelectedListener {
    static final String NAME = "name";
    static final String VALUE = "value";
    static final String OPTIONS = "options";

    ViewerActivity mViewerActivity;
    JSONArray mStateJson;

    public StateAdapter(ViewerActivity viewerActivity) {
        mViewerActivity = viewerActivity;
        try {
            mStateJson = new JSONArray("[{\"name\": \"Please\", " +
                    "\"value\": \"Initialize\", \"options\": []}]");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void setState(String stateJson) {
        try {
            mStateJson = new JSONArray(stateJson);
            notifyDataSetChanged();
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public int getCount() {
        return mStateJson.length();
    }

    @Override
    public Object getItem(int position) {
        try {
            return mStateJson.getJSONObject(position);
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = LayoutInflater.from(mViewerActivity).inflate(R.layout.state_item, null);
        }
        TextView nameText = (TextView) convertView.findViewById(R.id.nameText);
        TextView valueText = (TextView) convertView.findViewById(R.id.valueText);
        Spinner optionSpinner = (Spinner) convertView.findViewById(R.id.optionSpinner);
        JSONObject stateObject = (JSONObject) getItem(position);
        try {
            nameText.setText(stateObject.getString(NAME));
            String value = stateObject.getString(VALUE);
            JSONArray options = stateObject.getJSONArray(OPTIONS);
            if (options.length() == 0) {
                valueText.setText(value);
                valueText.setVisibility(View.VISIBLE);
                optionSpinner.setVisibility(View.GONE);

            } else {
                ArrayList<String> optionList = new ArrayList<>();
                String[] optionStrings = new String[options.length()];
                for(int i=0; i<options.length(); i++) {
                    optionList.add(options.getString(i));
                }
                optionSpinner.setAdapter(new ArrayAdapter<String>(mViewerActivity,
                        android.R.layout.simple_spinner_dropdown_item, optionList));
                optionSpinner.setSelection(optionList.indexOf(value));
                optionSpinner.setOnItemSelectedListener(this);
                optionSpinner.setVisibility(View.VISIBLE);
                valueText.setVisibility(View.GONE);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return convertView;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        View stateItem = (View) parent.getParent();
        String stateName = ((TextView) stateItem.findViewById(R.id.nameText)).getText().toString();
        String stateValue = ((TextView) view).getText().toString();
        mViewerActivity.onStateChanged(stateName, stateValue);
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // do nothing
    }
}
