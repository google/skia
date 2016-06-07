package org.skia.viewer;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

/*
    The navigation drawer requires ListView, so we implemented this BaseAdapter for that ListView.
    However, the ListView does not provide good support for updating just a single child view.
    For example, a frequently changed child view such as FPS state will reset the spinner of
    all other child views; although I didn't change other child views and directly return
    the convertView in BaseAdapter.getView(int position, View convertView, ViewGroup parent).

    Therefore, our adapter only returns one LinearLayout for the ListView.
    Within that LinearLayout, we maintain views ourselves so we can efficiently update its children.
 */
public class StateAdapter extends BaseAdapter implements AdapterView.OnItemSelectedListener {
    static final String NAME = "name";
    static final String VALUE = "value";
    static final String OPTIONS = "options";

    ViewerActivity mViewerActivity;
    LinearLayout mLayout;
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
            if (mLayout != null) {
                updateDrawer();
            } else {
                notifyDataSetChanged();
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public int getCount() {
        return 1;
    }

    @Override
    public Object getItem(int position) {
        return null;
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (mLayout == null) {
            mLayout = new LinearLayout(mViewerActivity);
            mLayout.setOrientation(LinearLayout.VERTICAL);
            updateDrawer();
        }
        return mLayout;
    }

    private View inflateItemView(JSONObject item) throws JSONException {
        View itemView = LayoutInflater.from(mViewerActivity).inflate(R.layout.state_item, null);
        TextView nameText = (TextView) itemView.findViewById(R.id.nameText);
        TextView valueText = (TextView) itemView.findViewById(R.id.valueText);
        Spinner optionSpinner = (Spinner) itemView.findViewById(R.id.optionSpinner);
        nameText.setText(item.getString(NAME));
        String value = item.getString(VALUE);
        JSONArray options = item.getJSONArray(OPTIONS);
        if (options.length() == 0) {
            valueText.setText(value);
            valueText.setVisibility(View.VISIBLE);
            optionSpinner.setVisibility(View.GONE);

        } else {
            ArrayList<String> optionList = new ArrayList<>();
            String[] optionStrings = new String[options.length()];
            for (int j = 0; j < options.length(); j++) {
                optionList.add(options.getString(j));
            }
            optionSpinner.setAdapter(new ArrayAdapter<String>(mViewerActivity,
                    android.R.layout.simple_spinner_dropdown_item, optionList));
            optionSpinner.setSelection(optionList.indexOf(value));
            optionSpinner.setOnItemSelectedListener(this);
            optionSpinner.setVisibility(View.VISIBLE);
            valueText.setVisibility(View.GONE);
        }
        itemView.setTag(item.toString()); // To save unnecessary view update
        itemView.setTag(R.integer.value_tag_key, value); // To save unnecessary state change event
        return itemView;
    }

    private void updateDrawer() {
        try {
            if (mStateJson.length() < mLayout.getChildCount()) {
                mLayout.removeViews(
                        mStateJson.length(), mLayout.getChildCount() - mStateJson.length());
            }
            for (int i = 0; i < mStateJson.length(); i++) {
                JSONObject stateObject = mStateJson.getJSONObject(i);
                if (mLayout.getChildCount() > i) {
                    View childView = mLayout.getChildAt(i);
                    if (stateObject.toString().equals(childView.getTag())) {
                        continue; // No update, reuse the old view and skip the remaining step
                    } else {
                        mLayout.removeViewAt(i);
                    }
                }
                mLayout.addView(inflateItemView(stateObject), i);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        View stateItem = (View) parent.getParent();
        String stateName = ((TextView) stateItem.findViewById(R.id.nameText)).getText().toString();
        String stateValue = ((TextView) view).getText().toString();
        if (!stateValue.equals(stateItem.getTag(R.integer.value_tag_key))) {
            stateItem.setTag(null); // Reset the tag to let updateDrawer update this item view.
            mViewerActivity.onStateChanged(stateName, stateValue);
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // do nothing
    }
}
