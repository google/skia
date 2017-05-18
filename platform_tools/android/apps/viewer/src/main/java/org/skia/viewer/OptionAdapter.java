package org.skia.viewer;

import android.content.Context;
import android.widget.ArrayAdapter;
import android.widget.Filter;
import android.widget.Spinner;

import java.util.ArrayList;
import java.util.List;

public class OptionAdapter extends ArrayAdapter<String> {
    private String mCurrentOption;
    private List<String> mOptions;
    private Filter mFilter = null;
    private Spinner mSpinner;

    public OptionAdapter(Context context, int resource, List<String> options, Spinner spinner) {
        super(context, resource);
        addAll(options);
        mOptions = options;
        mSpinner = spinner;
    }

    public void setCurrentOption(String currentOption) {
        this.mCurrentOption = currentOption;
    }

    private class OptionFilter extends Filter {

        @Override
        protected FilterResults performFiltering(CharSequence constraint) {
            final String pattern = constraint.toString().toLowerCase();
            ArrayList<String> filteredOptions = new ArrayList<>();
            for(String option : mOptions) {
                if (option.equals(mCurrentOption)
                        || option.toLowerCase().indexOf(pattern) > -1) {
                    filteredOptions.add(option);
                }
            }
            FilterResults results = new FilterResults();
            results.values = filteredOptions;
            results.count = filteredOptions.size();
            return results;
        }

        @Override
        protected void publishResults(CharSequence constraint, FilterResults results) {
            clear();
            List<String> filteredOptions = (List<String>) results.values;
            addAll(filteredOptions);
            // We set the selection to the current option to avoid unwanted option selection change
            mSpinner.setSelection(filteredOptions.indexOf(mCurrentOption));
            notifyDataSetChanged();
        }
    }

    @Override
    public Filter getFilter() {
        if (mFilter == null) {
            mFilter = new OptionFilter();
        }
        return mFilter;
    }
}
