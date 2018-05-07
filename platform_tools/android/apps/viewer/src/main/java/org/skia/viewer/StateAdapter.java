package org.skia.viewer;

import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.Switch;
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
    private static final String NAME = "name";
    private static final String VALUE = "value";
    private static final String OPTIONS = "options";
    private static final String BACKEND_STATE_NAME = "Backend";
    private static final String REFRESH_STATE_NAME = "Refresh";
    private static final String ON = "ON";
    private static final String OFF = "OFF";
    private static final int FILTER_LENGTH = 20;

    private ViewerActivity mViewerActivity;
    private LinearLayout mLayout;
    private JSONArray mStateJson;

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

    // The first list item is the mLayout that contains a list of state items
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
        switch (position) {
            case 0: {
                if (mLayout == null) {
                    mLayout = new LinearLayout(mViewerActivity);
                    mLayout.setOrientation(LinearLayout.VERTICAL);
                    updateDrawer();
                }
                return mLayout;
            }
            default: {
                return null;
            }
        }
    }

    private void populateView(JSONObject item, View view) throws JSONException {
        LinearLayout itemView = (LinearLayout) view;
        TextView nameText = (TextView) itemView.findViewById(R.id.nameText);
        TextView valueText = (TextView) itemView.findViewById(R.id.valueText);
        Spinner optionSpinner = (Spinner) itemView.findViewById(R.id.optionSpinner);

        String value = item.getString(VALUE);
        itemView.setTag(item.toString()); // To save unnecessary view update
        itemView.setTag(R.integer.value_tag_key, value);

        nameText.setText(item.getString(NAME));

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
            final OptionAdapter adapter = new OptionAdapter(mViewerActivity,
                    android.R.layout.simple_spinner_dropdown_item, optionList, optionSpinner);
            adapter.setCurrentOption(value);
            optionSpinner.setAdapter(adapter);
            if (optionStrings.length >= FILTER_LENGTH) {
                View existingView = itemView.getChildAt(1);
                if (!(existingView instanceof EditText)) {
                    EditText filterText = new EditText(mViewerActivity);
                    filterText.setHint("Filter");
                    itemView.addView(filterText, 1);
                    filterText.addTextChangedListener(new TextWatcher() {
                        @Override
                        public void beforeTextChanged(CharSequence s, int start, int cnt,
                                int after) {
                        }

                        @Override
                        public void onTextChanged(CharSequence s, int start, int before, int cnt) {
                        }

                        @Override
                        public void afterTextChanged(Editable s) {
                            adapter.getFilter().filter(s.toString());
                        }
                    });
                }
            }
            optionSpinner.setSelection(optionList.indexOf(value));
            optionSpinner.setOnItemSelectedListener(this);
            optionSpinner.setVisibility(View.VISIBLE);
            valueText.setVisibility(View.GONE);
        }
    }
    private View inflateItemView(JSONObject item) throws JSONException {
        LinearLayout itemView = (LinearLayout)
                LayoutInflater.from(mViewerActivity).inflate(R.layout.state_item, null);
        populateView(item, itemView);
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
                View childView = mLayout.getChildAt(i);
                if (childView != null) {
                    if (stateObject.toString().equals(childView.getTag())) {
                        continue; // No update, reuse the old view and skip the remaining step
                    } else {
                        populateView(stateObject, childView);
                    }
                } else {
                    mLayout.addView(inflateItemView(stateObject), i);
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        if (view == null) {
            return;
        }
        View stateItem = (View) parent.getParent();
        String stateName = ((TextView) stateItem.findViewById(R.id.nameText)).getText().toString();
        String stateValue = ((TextView) view).getText().toString();
        if (!stateValue.equals(stateItem.getTag(R.integer.value_tag_key))) {
            stateItem.setTag(null); // Reset the tag to let updateDrawer update this item view.
            mViewerActivity.onStateChanged(stateName, stateValue);
        }

        // Due to the current Android limitation, we're required to recreate the SurfaceView for
        // switching to/from the Raster backend.
        // (Although we can switch between GPU backend without recreating the SurfaceView.)
        final Object oldValue = stateItem.getTag(R.integer.value_tag_key);
        if (stateName.equals(BACKEND_STATE_NAME)
                && oldValue != null && !stateValue.equals(oldValue)) {
            LinearLayout mainLayout = (LinearLayout) mViewerActivity.findViewById(R.id.mainLayout);
            mainLayout.removeAllViews();
            SurfaceView surfaceView = new SurfaceView(mViewerActivity);
            surfaceView.setId(R.id.surfaceView);
            surfaceView.getHolder().addCallback(mViewerActivity);
            surfaceView.setOnTouchListener(mViewerActivity);
            mainLayout.addView(surfaceView);
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // do nothing
    }
}
